/**
 * This [node-gdal](https://github.com/naturalatlas/node-gdal) binding for
 * Node.js provides a feature-complete way of reading, writing, and manipulating
 * geospatial data (raster and vector) using [GDAL](http://www.gdal.org/).
 * ```
 * // sample: open a shapefile and display all features as geojson
 * var dataset = gdal.open("states.shp");
 *
 * dataset.layers.get(0).features.forEach(function(feature) {
 *     console.log(feature.getGeometry().toJSON());
 * });
 * ```
 *
 * <iframe src="http://ghbtns.com/github-btn.html?user=naturalatlas&repo=node-gdal&type=watch&count=true" allowtransparency="true" frameborder="0" scrolling="0" width="90" height="20" style="margin-top:20px"></iframe>
 * <iframe src="http://ghbtns.com/github-btn.html?user=naturalatlas&repo=node-gdal&type=fork&count=true" allowtransparency="true" frameborder="0" scrolling="0" width="200" height="20"></iframe>
 *
 * @class gdal
 */

// node
#include <node.h>
#include <node_buffer.h>
#include <node_version.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal.h>

// node-gdal
#include "gdal_common.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_warper.hpp"
#include "gdal_algorithms.hpp"

#include "gdal_layer.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_field_defn.hpp"
#include "gdal_feature.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_coordinate_transformation.hpp"
#include "gdal_point.hpp"
#include "gdal_polygon.hpp"
#include "gdal_linestring.hpp"
#include "gdal_linearring.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipoint.hpp"
#include "gdal_multilinestring.hpp"
#include "gdal_multipolygon.hpp"
#include "gdal_geometry.hpp"

#include "gdal.hpp"
#include "utils/field_types.hpp"

//collections
#include "collections/dataset_bands.hpp"
#include "collections/dataset_layers.hpp"
#include "collections/layer_features.hpp"
#include "collections/feature_fields.hpp"
#include "collections/layer_fields.hpp"
#include "collections/feature_defn_fields.hpp"
#include "collections/geometry_collection_children.hpp"
#include "collections/polygon_rings.hpp"
#include "collections/linestring_points.hpp"
#include "collections/rasterband_overviews.hpp"
#include "collections/rasterband_pixels.hpp"
#include "collections/gdal_drivers.hpp"

// std
#include <string>
#include <sstream>
#include <vector>

namespace node_gdal {

	using namespace node;
	using namespace v8;

	FILE *log_file = NULL;
	PtrManager ptr_manager;

	/**
	 * @attribute lastError
	 * @type Object
	 */
	static NAN_GETTER(LastErrorGetter)
	{
		Nan::HandleScope scope;

		int errtype = CPLGetLastErrorType();
		if (errtype == CE_None) {
			info.GetReturnValue().Set(Nan::Null());
			return;
		}

		Local<Object> result = Nan::New<Object>();
		Nan::Set(result, Nan::New("code").ToLocalChecked(), Nan::New(CPLGetLastErrorNo()));
		Nan::Set(result, Nan::New("message").ToLocalChecked(), Nan::New(CPLGetLastErrorMsg()).ToLocalChecked());
		Nan::Set(result, Nan::New("level").ToLocalChecked(), Nan::New(errtype));
		info.GetReturnValue().Set(result);
	}

	static NAN_SETTER(LastErrorSetter)
	{
		Nan::HandleScope scope;

		if (value->IsNull()) {
			CPLErrorReset();
		} else {
			Nan::ThrowError("'lastError' only supports being set to null");
			return;
		}
	}

	extern "C" {

		static NAN_METHOD(QuietOutput)
		{
			CPLSetErrorHandler(CPLQuietErrorHandler);
			return;
		}

		static NAN_METHOD(VerboseOutput)
		{
			CPLSetErrorHandler(CPLDefaultErrorHandler);
			return;
		}

		#ifdef ENABLE_LOGGING
		static NAN_GC_CALLBACK(beforeGC)
		{
			LOG("%s", "Starting garbage collection");
		}

		static NAN_GC_CALLBACK(afterGC)
		{
			LOG("%s", "Finished garbage collection");
		}
		#endif

		static NAN_METHOD(StartLogging)
		{
			Nan::HandleScope scope;

			#ifdef ENABLE_LOGGING
			std::string filename = "";
			NODE_ARG_STR(0, "filename", filename);
			if (filename.empty()) {
				Nan::ThrowError("Invalid filename");
				return;
			}
			if (log_file) fclose(log_file);
			log_file = fopen(filename.c_str(), "w");
			if (!log_file) {
				Nan::ThrowError("Error creating log file");
				return;
			}

  			Nan::AddGCPrologueCallback(beforeGC);
  			Nan::AddGCEpilogueCallback(afterGC);

			#else
			Nan::ThrowError("Logging requires node-gdal be compiled with --enable_logging=true");
			#endif

			return;
		}

		static NAN_METHOD(StopLogging)
		{
			#ifdef ENABLE_LOGGING
			if (log_file) {
				fclose(log_file);
				log_file = NULL;
			}
			#endif

			return;
		}

		static NAN_METHOD(Log)
		{
			Nan::HandleScope scope;
			std::string msg;
			NODE_ARG_STR(0, "message", msg);
			msg = msg + "\n";

			#ifdef ENABLE_LOGGING
			if (log_file) {
				fputs(msg.c_str(), log_file);
				fflush(log_file);
			}
			#endif

			return;
		}

		static NAN_METHOD(ThrowDummyCPLError)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "Mock error");
			return;
		}

		static NAN_METHOD(isAlive)
		{
			Nan::HandleScope scope;

			long uid;
			NODE_ARG_INT(0, "uid", uid);

			info.GetReturnValue().Set(Nan::New(ptr_manager.isAlive(uid)));
		}

		static void Init(Local<Object> target)
		{

			Nan::SetMethod(target, "open", open);
			Nan::SetMethod(target, "setConfigOption", setConfigOption);
			Nan::SetMethod(target, "getConfigOption", getConfigOption);
			Nan::SetMethod(target, "decToDMS", decToDMS);
			Nan::SetMethod(target, "_triggerCPLError", ThrowDummyCPLError); // for tests
			Nan::SetMethod(target, "_isAlive", isAlive); // for tests

			Warper::Initialize(target);
			Algorithms::Initialize(target);

			Driver::Initialize(target);
			Dataset::Initialize(target);
			RasterBand::Initialize(target);

			Layer::Initialize(target);
			Feature::Initialize(target);
			FeatureDefn::Initialize(target);
			FieldDefn::Initialize(target);
			Geometry::Initialize(target);
			Point::Initialize(target);
			LineString::Initialize(target);
			LinearRing::Initialize(target);
			Polygon::Initialize(target);
			GeometryCollection::Initialize(target);
			MultiPoint::Initialize(target);
			MultiLineString::Initialize(target);
			MultiPolygon::Initialize(target);
			SpatialReference::Initialize(target);
			CoordinateTransformation::Initialize(target);

			DatasetBands::Initialize(target);
			DatasetLayers::Initialize(target);
			LayerFeatures::Initialize(target);
			FeatureFields::Initialize(target);
			LayerFields::Initialize(target);
			FeatureDefnFields::Initialize(target);
			GeometryCollectionChildren::Initialize(target);
			PolygonRings::Initialize(target);
			LineStringPoints::Initialize(target);
			RasterBandOverviews::Initialize(target);
			RasterBandPixels::Initialize(target);

			/**
			 * The collection of all drivers registered with GDAL
			 *
			 * @final
			 * @for gdal
			 * @property gdal.drivers
			 * @type {gdal.GDALDrivers}
			 */
			GDALDrivers::Initialize(target); //calls GDALRegisterAll()
			Nan::Set(target, Nan::New("drivers").ToLocalChecked(), GDALDrivers::New());

			/**
			 * @class Constants (DMD)
			 */

			/**
			 * @final
			 * @property gdal.DMD_LONGNAME
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_LONGNAME").ToLocalChecked(), Nan::New(GDAL_DMD_LONGNAME).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DMD_MIMETYPE
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_MIMETYPE").ToLocalChecked(), Nan::New(GDAL_DMD_MIMETYPE).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DMD_HELPTOPIC
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_HELPTOPIC").ToLocalChecked(), Nan::New(GDAL_DMD_HELPTOPIC).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DMD_EXTENSION
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_EXTENSION").ToLocalChecked(), Nan::New(GDAL_DMD_EXTENSION).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DMD_CREATIONOPTIONLIST
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_CREATIONOPTIONLIST").ToLocalChecked(), Nan::New(GDAL_DMD_CREATIONOPTIONLIST).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DMD_CREATIONDATATYPES
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DMD_CREATIONDATATYPES").ToLocalChecked(), Nan::New(GDAL_DMD_CREATIONDATATYPES).ToLocalChecked());

			/**
			 * @class Constants (CPL Error Levels)
			 */

			/**
			 * Error level: (no error)
			 *
			 * @final
			 * @property gdal.CE_None
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CE_None").ToLocalChecked(), Nan::New(CE_None));
			/**
			 * Error level: Debug
			 *
			 * @final
			 * @property gdal.CE_Debug
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CE_Debug").ToLocalChecked(), Nan::New(CE_Debug));
			/**
			 * Error level: Warning
			 *
			 * @final
			 * @property gdal.CE_Warning
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CE_Warning").ToLocalChecked(), Nan::New(CE_Warning));
			/**
			 * Error level: Failure
			 *
			 * @final
			 * @property gdal.CE_Failure
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CE_Failure").ToLocalChecked(), Nan::New(CE_Failure));
			/**
			 * Error level: Fatal
			 *
			 * @property gdal.CE_Fatal
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CE_Fatal").ToLocalChecked(), Nan::New(CE_Fatal));


			/**
			 * @class Constants (CPL Error Codes)
			 */

			/**
			 * @final
			 * @property gdal.CPLE_None
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_None").ToLocalChecked(), Nan::New(CPLE_None));
			/**
			 * @final
			 * @property gdal.CPLE_AppDefined
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_AppDefined").ToLocalChecked(), Nan::New(CPLE_AppDefined));
			/**
			 * @final
			 * @property gdal.CPLE_OutOfMemory
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_OutOfMemory").ToLocalChecked(), Nan::New(CPLE_OutOfMemory));
			/**
			 * @final
			 * @property gdal.CPLE_FileIO
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_FileIO").ToLocalChecked(), Nan::New(CPLE_FileIO));
			/**
			 * @final
			 * @property gdal.CPLE_OpenFailed
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_OpenFailed").ToLocalChecked(), Nan::New(CPLE_OpenFailed));
			/**
			 * @final
			 * @property gdal.CPLE_IllegalArg
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_IllegalArg").ToLocalChecked(), Nan::New(CPLE_IllegalArg));
			/**
			 * @final
			 * @property gdal.CPLE_NotSupported
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_NotSupported").ToLocalChecked(), Nan::New(CPLE_NotSupported));
			/**
			 * @final
			 * @property gdal.CPLE_AssertionFailed
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_AssertionFailed").ToLocalChecked(), Nan::New(CPLE_AssertionFailed));
			/**
			 * @final
			 * @property gdal.CPLE_NoWriteAccess
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_NoWriteAccess").ToLocalChecked(), Nan::New(CPLE_NoWriteAccess));
			/**
			 * @final
			 * @property gdal.CPLE_UserInterrupt
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_UserInterrupt").ToLocalChecked(), Nan::New(CPLE_UserInterrupt));
			/**
			 * @final
			 * @property gdal.CPLE_ObjectNull
			 * @type {Number}
			 */
			Nan::Set(target, Nan::New("CPLE_ObjectNull").ToLocalChecked(), Nan::New(CPLE_ObjectNull));

			/**
			 * @class Constants (DCAP)
			 */

			/**
			 * @final
			 * @property gdal.DCAP_CREATE
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DCAP_CREATE").ToLocalChecked(), Nan::New(GDAL_DCAP_CREATE).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DCAP_CREATECOPY
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DCAP_CREATECOPY").ToLocalChecked(), Nan::New(GDAL_DCAP_CREATECOPY).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.DCAP_VIRTUALIO
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("DCAP_VIRTUALIO").ToLocalChecked(), Nan::New(GDAL_DCAP_VIRTUALIO).ToLocalChecked());

			/**
			 * @class Constants (OLC)
			 */

			/**
			 * @final
			 * @property gdal.OLCRandomRead
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCRandomRead").ToLocalChecked(), Nan::New(OLCRandomRead).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCSequentialWrite
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCSequentialWrite").ToLocalChecked(), Nan::New(OLCSequentialWrite).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCRandomWrite
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCRandomWrite").ToLocalChecked(), Nan::New(OLCRandomWrite).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCFastSpatialFilter
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCFastSpatialFilter").ToLocalChecked(), Nan::New(OLCFastSpatialFilter).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCFastFeatureCount
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCFastFeatureCount").ToLocalChecked(), Nan::New(OLCFastFeatureCount).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCFastGetExtent
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCFastGetExtent").ToLocalChecked(), Nan::New(OLCFastGetExtent).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCCreateField
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCCreateField").ToLocalChecked(), Nan::New(OLCCreateField).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCDeleteField
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCDeleteField").ToLocalChecked(), Nan::New(OLCDeleteField).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCReorderFields
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCReorderFields").ToLocalChecked(), Nan::New(OLCReorderFields).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCAlterFieldDefn
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCAlterFieldDefn").ToLocalChecked(), Nan::New(OLCAlterFieldDefn).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCTransactions
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCTransactions").ToLocalChecked(), Nan::New(OLCTransactions).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCDeleteFeature
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCDeleteFeature").ToLocalChecked(), Nan::New(OLCDeleteFeature).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCFastSetNextByIndex
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCFastSetNextByIndex").ToLocalChecked(), Nan::New(OLCFastSetNextByIndex).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCStringsAsUTF8
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCStringsAsUTF8").ToLocalChecked(), Nan::New(OLCStringsAsUTF8).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OLCIgnoreFields
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCIgnoreFields").ToLocalChecked(), Nan::New(OLCIgnoreFields).ToLocalChecked());

			#ifdef OLCCreateGeomField
			/**
			 * @final
			 * @property gdal.OLCCreateGeomField
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OLCCreateGeomField").ToLocalChecked(), Nan::New(OLCCreateGeomField).ToLocalChecked());
			#endif
			#ifdef ODsCCreateGeomFieldAfterCreateLayer

			/**
			 * @class Constants (ODsC)
			 */

			/**
			 * @final
			 * @property gdal.ODsCCreateLayer
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("ODsCCreateLayer").ToLocalChecked(), Nan::New(ODsCCreateLayer).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.ODsCDeleteLayer
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("ODsCDeleteLayer").ToLocalChecked(), Nan::New(ODsCDeleteLayer).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.ODsCCreateGeomFieldAfterCreateLayer
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("ODsCCreateGeomFieldAfterCreateLayer").ToLocalChecked(), Nan::New(ODsCCreateGeomFieldAfterCreateLayer).ToLocalChecked());
			#endif
			/**
			 * @final
			 * @property gdal.ODrCCreateDataSource
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("ODrCCreateDataSource").ToLocalChecked(), Nan::New(ODrCCreateDataSource).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.ODrCDeleteDataSource
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("ODrCDeleteDataSource").ToLocalChecked(), Nan::New(ODrCDeleteDataSource).ToLocalChecked());

			NODE_DEFINE_CONSTANT(target, GA_ReadOnly);
			NODE_DEFINE_CONSTANT(target, GA_Update);
			NODE_DEFINE_CONSTANT(target, GF_Read);
			NODE_DEFINE_CONSTANT(target, GF_Write);

			/**
			 * Pixel data types.
			 *
			 * @class Constants (GDT)
			 */

			/**
			 * Unknown or unspecified type
			 * @final
			 * @property gdal.GDT_Unknown
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Unknown").ToLocalChecked(), Nan::Undefined());
			/**
			 * Eight bit unsigned integer
			 * @final
			 * @property gdal.GDT_Byte
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Byte").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_Byte)).ToLocalChecked());
			/**
			 * Sixteen bit unsigned integer
			 * @final
			 * @property gdal.GDT_UInt16
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_UInt16").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_UInt16)).ToLocalChecked());
			/**
			 * Sixteen bit signed integer
			 * @final
			 * @property gdal.GDT_Int16
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Int16").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_Int16)).ToLocalChecked());
			/**
			 * Thirty two bit unsigned integer
			 * @final
			 * @property gdal.GDT_UInt32
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_UInt32").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_UInt32)).ToLocalChecked());
			/**
			 * Thirty two bit signed integer
			 * @final
			 * @property gdal.GDT_Int32
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Int32").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_Int32)).ToLocalChecked());
			/**
			 * Thirty two bit floating point
			 * @final
			 * @property gdal.GDT_Float32
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Float32").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_Float32)).ToLocalChecked());
			/**
			 * Sixty four bit floating point
			 * @final
			 * @property gdal.GDT_Float64
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_Float64").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_Float64)).ToLocalChecked());
			/**
			 * Complex Int16
			 * @final
			 * @property gdal.GDT_CInt16
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_CInt16").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_CInt16)).ToLocalChecked());
			/**
			 * Complex Int32
			 * @final
			 * @property gdal.GDT_CInt32
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_CInt32").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_CInt32)).ToLocalChecked());
			/**
			 * Complex Float32
			 * @final
			 * @property gdal.GDT_CFloat32
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_CFloat32").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_CFloat32)).ToLocalChecked());
			/**
			 * Complex Float64
			 * @final
			 * @property gdal.GDT_CFloat64
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GDT_CFloat64").ToLocalChecked(), Nan::New(GDALGetDataTypeName(GDT_CFloat64)).ToLocalChecked());

			/**
			 * @class Constants (OJ)
			 */

			/**
			 * @final
			 * @property gdal.OJUndefined
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OJUndefined").ToLocalChecked(), Nan::Undefined());
			/**
			 * @final
			 * @property gdal.OJLeft
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OJLeft").ToLocalChecked(), Nan::New("Left").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OJRight
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OJRight").ToLocalChecked(), Nan::New("Right").ToLocalChecked());

			/**
			 * @class Constants (GCI)
			 */

			/**
			 * @final
			 * @property gdal.GCI_Undefined
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_Undefined").ToLocalChecked(), Nan::Undefined());
			/**
			 * @final
			 * @property gdal.GCI_GrayIndex
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_GrayIndex").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_GrayIndex)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_PaletteIndex
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_PaletteIndex").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_PaletteIndex)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_RedBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_RedBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_RedBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_GreenBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_GreenBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_GreenBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_BlueBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_BlueBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_BlueBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_AlphaBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_AlphaBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_AlphaBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_HueBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_HueBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_HueBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_SaturationBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_SaturationBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_SaturationBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_LightnessBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_LightnessBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_LightnessBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_CyanBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_CyanBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_CyanBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_MagentaBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_MagentaBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_MagentaBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_YellowBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_YellowBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_YellowBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_BlackBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_BlackBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_BlackBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_YBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_YCbCr_YBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_YCbCr_YBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_CbBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_YCbCr_CbBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_YCbCr_CbBand)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_CrBand
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GCI_YCbCr_CrBand").ToLocalChecked(), Nan::New(GDALGetColorInterpretationName(GCI_YCbCr_CrBand)).ToLocalChecked());

			/**
			 * @class Constants (wkbVariant)
			 */

			/**
			 * Old-style 99-402 extended dimension (Z) WKB types.
			 * Synonymous with 'wkbVariantOldOgc' (gdal >= 2.0)
			 *
			 * @final
			 * @property gdal.wkbVariantOgc
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbVariantOgc").ToLocalChecked(), Nan::New("OGC").ToLocalChecked());

			/**
			 * Old-style 99-402 extended dimension (Z) WKB types.
			 * Synonymous with 'wkbVariantOgc' (gdal < 2.0)
			 *
			 * @final
			 * @property gdal.wkbVariantOldOgc
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbVariantOldOgc").ToLocalChecked(), Nan::New("OGC").ToLocalChecked());

			/**
			 * SFSQL 1.2 and ISO SQL/MM Part 3 extended dimension (Z&M) WKB types.
			 *
			 * @final
			 * @property gdal.wkbVariantIso
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbVariantIso").ToLocalChecked(), Nan::New("ISO").ToLocalChecked());

			/**
			 * @class Constants (wkbByteOrder)
			 */

			/**
			 * @final
			 * @property gdal.wkbXDR
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbXDR").ToLocalChecked(), Nan::New("MSB").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.wkbNDR
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbNDR").ToLocalChecked(), Nan::New("LSB").ToLocalChecked());

			/**
			 * @class Constants (wkbGeometryType)
			 */

			/**
			 * @final
			 *
			 * The `wkb25DBit` constant can be used to convert between 2D types to 2.5D types
			 *
			 * @example
			 * ```
			 * // 2 -> 2.5D
			 * wkbPoint25D = gdal.wkbPoint | gdal.wkb25DBit
			 *
			 * // 2.5D -> 2D (same as wkbFlatten())
			 * wkbPoint = gdal.wkbPoint25D & (~gdal.wkb25DBit)```
			 *
			 * @property gdal.wkb25DBit
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkb25DBit").ToLocalChecked(), Nan::New<Integer>(wkb25DBit));

			int wkbLinearRing25D = wkbLinearRing | wkb25DBit;

			/**
			 * @final
			 * @property gdal.wkbUnknown
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbUnknown").ToLocalChecked(), Nan::New<Integer>(wkbUnknown));
			/**
			 * @final
			 * @property gdal.wkbPoint
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbPoint").ToLocalChecked(), Nan::New<Integer>(wkbPoint));
			/**
			 * @final
			 * @property gdal.wkbLineString
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbLineString").ToLocalChecked(), Nan::New<Integer>(wkbLineString));
			/**
			 * @final
			 * @property gdal.wkbPolygon
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbPolygon").ToLocalChecked(), Nan::New<Integer>(wkbPolygon));
			/**
			 * @final
			 * @property gdal.wkbMultiPoint
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiPoint").ToLocalChecked(), Nan::New<Integer>(wkbMultiPoint));
			/**
			 * @final
			 * @property gdal.wkbMultiLineString
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiLineString").ToLocalChecked(), Nan::New<Integer>(wkbMultiLineString));
			/**
			 * @final
			 * @property gdal.wkbMultiPolygon
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiPolygon").ToLocalChecked(), Nan::New<Integer>(wkbMultiPolygon));
			/**
			 * @final
			 * @property gdal.wkbGeometryCollection
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbGeometryCollection").ToLocalChecked(), Nan::New<Integer>(wkbGeometryCollection));
			/**
			 * @final
			 * @property gdal.wkbNone
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbNone").ToLocalChecked(), Nan::New<Integer>(wkbNone));
			/**
			 * @final
			 * @property gdal.wkbLinearRing
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("wkbLinearRing").ToLocalChecked(), Nan::New<Integer>(wkbLinearRing));
			/**
			 * @final
			 * @property gdal.wkbPoint25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbPoint25D").ToLocalChecked(), Nan::New<Integer>(wkbPoint25D));
			/**
			 * @final
			 * @property gdal.wkbLineString25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbLineString25D").ToLocalChecked(), Nan::New<Integer>(wkbLineString25D));
			/**
			 * @final
			 * @property gdal.wkbPolygon25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbPolygon25D").ToLocalChecked(), Nan::New<Integer>(wkbPolygon25D));
			/**
			 * @final
			 * @property gdal.wkbMultiPoint25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiPoint25D").ToLocalChecked(), Nan::New<Integer>(wkbMultiPoint25D));
			/**
			 * @final
			 * @property gdal.wkbMultiLineString25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiLineString25D").ToLocalChecked(), Nan::New<Integer>(wkbMultiLineString25D));
			/**
			 * @final
			 * @property gdal.wkbMultiPolygon25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbMultiPolygon25D").ToLocalChecked(), Nan::New<Integer>(wkbMultiPolygon25D));
			/**
			 * @final
			 * @property gdal.wkbGeometryCollection25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbGeometryCollection25D").ToLocalChecked(), Nan::New<Integer>(wkbGeometryCollection25D));
			/**
			 * @final
			 * @property gdal.wkbLinearRing25D
			 * @type {integer}
			 */
			Nan::Set(target, Nan::New("wkbLinearRing25D").ToLocalChecked(), Nan::New<Integer>(wkbLinearRing25D));

			/**
			 * @class Constants (OFT)
			 */

			/**
			 * @final
			 * @property gdal.OFTInteger
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTInteger").ToLocalChecked(), Nan::New(getFieldTypeName(OFTInteger)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTIntegerList
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTIntegerList").ToLocalChecked(), Nan::New(getFieldTypeName(OFTIntegerList)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTReal
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTReal").ToLocalChecked(), Nan::New(getFieldTypeName(OFTReal)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTRealList
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTRealList").ToLocalChecked(), Nan::New(getFieldTypeName(OFTRealList)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTString
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTString").ToLocalChecked(), Nan::New(getFieldTypeName(OFTString)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTStringList
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTStringList").ToLocalChecked(), Nan::New(getFieldTypeName(OFTStringList)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTWideString
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTWideString").ToLocalChecked(), Nan::New(getFieldTypeName(OFTWideString)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTWideStringList
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTWideStringList").ToLocalChecked(), Nan::New(getFieldTypeName(OFTWideStringList)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTBinary
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTBinary").ToLocalChecked(), Nan::New(getFieldTypeName(OFTBinary)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTDate
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTDate").ToLocalChecked(), Nan::New(getFieldTypeName(OFTDate)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTTime
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTTime").ToLocalChecked(), Nan::New(getFieldTypeName(OFTTime)).ToLocalChecked());
			/**
			 * @final
			 * @property gdal.OFTDateTime
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("OFTDateTime").ToLocalChecked(), Nan::New(getFieldTypeName(OFTDateTime)).ToLocalChecked());

			/**
			 * Resampling options that can be used with the gdal.reprojectImage() method.
			 *
			 * @class Constants (GRA)
			 */

			/**
			 * @final
			 * @property gdal.GRA_NearestNeighbor
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_NearestNeighbor").ToLocalChecked(), Nan::New("NearestNeighbor").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_Bilinear
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_Bilinear").ToLocalChecked(), Nan::New("Bilinear").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_Cubic
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_Cubic").ToLocalChecked(), Nan::New("Cubic").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_CubicSpline
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_CubicSpline").ToLocalChecked(), Nan::New("CubicSpline").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_Lanczos
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_Lanczos").ToLocalChecked(), Nan::New("Lanczos").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_Average
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_Average").ToLocalChecked(), Nan::New("Average").ToLocalChecked());
			/**
			 * @final
			 * @property gdal.GRA_Mode
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("GRA_Mode").ToLocalChecked(), Nan::New("Mode").ToLocalChecked());

			/**
			 * GDAL version (not the binding version)
			 *
			 * @final
			 * @for gdal
			 * @property gdal.version
			 * @type {String}
			 */
			Nan::Set(target, Nan::New("version").ToLocalChecked(), Nan::New(GDAL_RELEASE_NAME).ToLocalChecked());

			/**
			 * Details about the last error that occurred. The property
			 * will be null or an object containing three properties: "number",
			 * "message", and "type".
			 *
			 * @final
			 * @for gdal
			 * @property gdal.lastError
			 * @type {Object}
			 */
			Nan::SetAccessor(target, Nan::New<v8::String>("lastError").ToLocalChecked(), LastErrorGetter, LastErrorSetter);

			// Local<Object> versions = Nan::New<Object>();
			// Nan::Set(versions, Nan::New("node").ToLocalChecked(), Nan::New(NODE_VERSION+1));
			// Nan::Set(versions, Nan::New("v8").ToLocalChecked(), Nan::New(V8::GetVersion()));
			// Nan::Set(target, Nan::New("versions").ToLocalChecked(), versions);

			/**
			 * Disables all output.
			 *
			 * @for gdal
			 * @static
			 * @method quiet
			 */
			Nan::SetMethod(target, "quiet", QuietOutput);

			/**
			 * Displays extra debugging information from GDAL.
			 *
			 * @for gdal
			 * @static
			 * @method verbose
			 */
			Nan::SetMethod(target, "verbose", VerboseOutput);

			Nan::SetMethod(target, "startLogging", StartLogging);
			Nan::SetMethod(target, "stopLogging", StopLogging);
			Nan::SetMethod(target, "log", Log);

			Local<Object> supports = Nan::New<Object>();
			Nan::Set(target, Nan::New("supports").ToLocalChecked(), supports);

			NODE_DEFINE_CONSTANT(target, CPLE_OpenFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_IllegalArg);
			NODE_DEFINE_CONSTANT(target, CPLE_NotSupported);
			NODE_DEFINE_CONSTANT(target, CPLE_AssertionFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_NoWriteAccess);
			NODE_DEFINE_CONSTANT(target, CPLE_UserInterrupt);

		}
	}

} // namespace node_gdal

NODE_MODULE(gdal, node_gdal::Init)
