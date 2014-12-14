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
#include "gdal_majorobject.hpp"
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

	extern "C" {

		static NAN_METHOD(QuietOutput)
		{
			CPLSetErrorHandler(CPLQuietErrorHandler);
			NanReturnUndefined();
		}

		static NAN_METHOD(VerboseOutput)
		{
			CPLSetErrorHandler(CPLDefaultErrorHandler);
			NanReturnUndefined();
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
			NanScope();

			#ifdef ENABLE_LOGGING
			std::string filename = "";
			NODE_ARG_STR(0, "filename", filename);
			if (filename.empty()) {
				NanThrowError("Invalid filename");
				NanReturnUndefined();
			}
			if (log_file) fclose(log_file);
			log_file = fopen(filename.c_str(), "w");
			if (!log_file) {
				NanThrowError("Error creating log file");
				NanReturnUndefined();
			}

  			NanAddGCPrologueCallback(beforeGC);
  			NanAddGCEpilogueCallback(afterGC);

			#else
			NanThrowError("Logging requires node-gdal be compiled with --enable_logging=true");
			#endif

			NanReturnUndefined();
		}

		static NAN_METHOD(StopLogging)
		{
			#ifdef ENABLE_LOGGING
			if (log_file) {
				fclose(log_file);
				log_file = NULL;
			}
			#endif

			NanReturnUndefined();
		}

		static NAN_METHOD(Log)
		{
			NanScope();
			std::string msg;
			NODE_ARG_STR(0, "message", msg);
			msg = msg + "\n";

			#ifdef ENABLE_LOGGING
			if (log_file) { 
				fputs(msg.c_str(), log_file); 
				fflush(log_file); 
			}
			#endif
			
			NanReturnUndefined();
		}

		static void Init(Handle<Object> target)
		{

			NODE_SET_METHOD(target, "open", open);
			NODE_SET_METHOD(target, "setConfigOption", setConfigOption);
			NODE_SET_METHOD(target, "getConfigOption", getConfigOption);
			NODE_SET_METHOD(target, "decToDMS", decToDMS);

			Warper::Initialize(target);
			Algorithms::Initialize(target);

			MajorObject::Initialize(target);
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
			target->Set(NanNew("drivers"), GDALDrivers::New());

			/**
			 * @class Constants (DMD)
			 */

			/**
			 * @final
			 * @property gdal.DMD_LONGNAME
			 * @type {String}
			 */
			target->Set(NanNew("DMD_LONGNAME"), NanNew(GDAL_DMD_LONGNAME));
			/**
			 * @final
			 * @property gdal.DMD_MIMETYPE
			 * @type {String}
			 */
			target->Set(NanNew("DMD_MIMETYPE"), NanNew(GDAL_DMD_MIMETYPE));
			/**
			 * @final
			 * @property gdal.DMD_HELPTOPIC
			 * @type {String}
			 */
			target->Set(NanNew("DMD_HELPTOPIC"), NanNew(GDAL_DMD_HELPTOPIC));
			/**
			 * @final
			 * @property gdal.DMD_EXTENSION
			 * @type {String}
			 */
			target->Set(NanNew("DMD_EXTENSION"), NanNew(GDAL_DMD_EXTENSION));
			/**
			 * @final
			 * @property gdal.DMD_CREATIONOPTIONLIST
			 * @type {String}
			 */
			target->Set(NanNew("DMD_CREATIONOPTIONLIST"), NanNew(GDAL_DMD_CREATIONOPTIONLIST));
			/**
			 * @final
			 * @property gdal.DMD_CREATIONDATATYPES
			 * @type {String}
			 */
			target->Set(NanNew("DMD_CREATIONDATATYPES"), NanNew(GDAL_DMD_CREATIONDATATYPES));

			/**
			 * @class Constants (DCAP)
			 */

			/**
			 * @final
			 * @property gdal.DCAP_CREATE
			 * @type {String}
			 */
			target->Set(NanNew("DCAP_CREATE"), NanNew(GDAL_DCAP_CREATE));
			/**
			 * @final
			 * @property gdal.DCAP_CREATECOPY
			 * @type {String}
			 */
			target->Set(NanNew("DCAP_CREATECOPY"), NanNew(GDAL_DCAP_CREATECOPY));
			/**
			 * @final
			 * @property gdal.DCAP_VIRTUALIO
			 * @type {String}
			 */
			target->Set(NanNew("DCAP_VIRTUALIO"), NanNew(GDAL_DCAP_VIRTUALIO));

			/**
			 * @class Constants (OLC)
			 */

			/**
			 * @final
			 * @property gdal.OLCRandomRead
			 * @type {String}
			 */
			target->Set(NanNew("OLCRandomRead"), NanNew(OLCRandomRead));
			/**
			 * @final
			 * @property gdal.OLCSequentialWrite
			 * @type {String}
			 */
			target->Set(NanNew("OLCSequentialWrite"), NanNew(OLCSequentialWrite));
			/**
			 * @final
			 * @property gdal.OLCRandomWrite
			 * @type {String}
			 */
			target->Set(NanNew("OLCRandomWrite"), NanNew(OLCRandomWrite));
			/**
			 * @final
			 * @property gdal.OLCFastSpatialFilter
			 * @type {String}
			 */
			target->Set(NanNew("OLCFastSpatialFilter"), NanNew(OLCFastSpatialFilter));
			/**
			 * @final
			 * @property gdal.OLCFastFeatureCount
			 * @type {String}
			 */
			target->Set(NanNew("OLCFastFeatureCount"), NanNew(OLCFastFeatureCount));
			/**
			 * @final
			 * @property gdal.OLCFastGetExtent
			 * @type {String}
			 */
			target->Set(NanNew("OLCFastGetExtent"), NanNew(OLCFastGetExtent));
			/**
			 * @final
			 * @property gdal.OLCCreateField
			 * @type {String}
			 */
			target->Set(NanNew("OLCCreateField"), NanNew(OLCCreateField));
			/**
			 * @final
			 * @property gdal.OLCDeleteField
			 * @type {String}
			 */
			target->Set(NanNew("OLCDeleteField"), NanNew(OLCDeleteField));
			/**
			 * @final
			 * @property gdal.OLCReorderFields
			 * @type {String}
			 */
			target->Set(NanNew("OLCReorderFields"), NanNew(OLCReorderFields));
			/**
			 * @final
			 * @property gdal.OLCAlterFieldDefn
			 * @type {String}
			 */
			target->Set(NanNew("OLCAlterFieldDefn"), NanNew(OLCAlterFieldDefn));
			/**
			 * @final
			 * @property gdal.OLCTransactions
			 * @type {String}
			 */
			target->Set(NanNew("OLCTransactions"), NanNew(OLCTransactions));
			/**
			 * @final
			 * @property gdal.OLCDeleteFeature
			 * @type {String}
			 */
			target->Set(NanNew("OLCDeleteFeature"), NanNew(OLCDeleteFeature));
			/**
			 * @final
			 * @property gdal.OLCFastSetNextByIndex
			 * @type {String}
			 */
			target->Set(NanNew("OLCFastSetNextByIndex"), NanNew(OLCFastSetNextByIndex));
			/**
			 * @final
			 * @property gdal.OLCStringsAsUTF8
			 * @type {String}
			 */
			target->Set(NanNew("OLCStringsAsUTF8"), NanNew(OLCStringsAsUTF8));
			/**
			 * @final
			 * @property gdal.OLCIgnoreFields
			 * @type {String}
			 */
			target->Set(NanNew("OLCIgnoreFields"), NanNew(OLCIgnoreFields));

			#ifdef OLCCreateGeomField
			/**
			 * @final
			 * @property gdal.OLCCreateGeomField
			 * @type {String}
			 */
			target->Set(NanNew("OLCCreateGeomField"), NanNew(OLCCreateGeomField));
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
			target->Set(NanNew("ODsCCreateLayer"), NanNew(ODsCCreateLayer));
			/**
			 * @final
			 * @property gdal.ODsCDeleteLayer
			 * @type {String}
			 */
			target->Set(NanNew("ODsCDeleteLayer"), NanNew(ODsCDeleteLayer));
			/**
			 * @final
			 * @property gdal.ODsCCreateGeomFieldAfterCreateLayer
			 * @type {String}
			 */
			target->Set(NanNew("ODsCCreateGeomFieldAfterCreateLayer"), NanNew(ODsCCreateGeomFieldAfterCreateLayer));
			#endif
			/**
			 * @final
			 * @property gdal.ODrCCreateDataSource
			 * @type {String}
			 */
			target->Set(NanNew("ODrCCreateDataSource"), NanNew(ODrCCreateDataSource));
			/**
			 * @final
			 * @property gdal.ODrCDeleteDataSource
			 * @type {String}
			 */
			target->Set(NanNew("ODrCDeleteDataSource"), NanNew(ODrCDeleteDataSource));

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
			target->Set(NanNew("GDT_Unknown"), NanUndefined());
			/**
			 * Eight bit unsigned integer
			 * @final
			 * @property gdal.GDT_Byte
			 * @type {String}
			 */
			target->Set(NanNew("GDT_Byte"), NanNew(GDALGetDataTypeName(GDT_Byte)));
			/**
			 * Sixteen bit unsigned integer
			 * @final
			 * @property gdal.GDT_UInt16
			 * @type {String}
			 */
			target->Set(NanNew("GDT_UInt16"), NanNew(GDALGetDataTypeName(GDT_UInt16)));
			/**
			 * Sixteen bit signed integer
			 * @final
			 * @property gdal.GDT_Int16
			 * @type {String}
			 */
			target->Set(NanNew("GDT_Int16"), NanNew(GDALGetDataTypeName(GDT_Int16)));
			/**
			 * Thirty two bit unsigned integer
			 * @final
			 * @property gdal.GDT_UInt32
			 * @type {String}
			 */
			target->Set(NanNew("GDT_UInt32"), NanNew(GDALGetDataTypeName(GDT_UInt32)));
			/**
			 * Thirty two bit signed integer
			 * @final
			 * @property gdal.GDT_Int32
			 * @type {String}
			 */
			target->Set(NanNew("GDT_Int32"), NanNew(GDALGetDataTypeName(GDT_Int32)));
			/**
			 * Thirty two bit floating point
			 * @final
			 * @property gdal.GDT_Float32
			 * @type {String}
			 */
			target->Set(NanNew("GDT_Float32"), NanNew(GDALGetDataTypeName(GDT_Float32)));
			/**
			 * Sixty four bit floating point
			 * @final
			 * @property gdal.GDT_Float64
			 * @type {String}
			 */
			target->Set(NanNew("GDT_Float64"), NanNew(GDALGetDataTypeName(GDT_Float64)));
			/**
			 * Complex Int16
			 * @final
			 * @property gdal.GDT_CInt16
			 * @type {String}
			 */
			target->Set(NanNew("GDT_CInt16"), NanNew(GDALGetDataTypeName(GDT_CInt16)));
			/**
			 * Complex Int32
			 * @final
			 * @property gdal.GDT_CInt32
			 * @type {String}
			 */
			target->Set(NanNew("GDT_CInt32"), NanNew(GDALGetDataTypeName(GDT_CInt32)));
			/**
			 * Complex Float32
			 * @final
			 * @property gdal.GDT_CFloat32
			 * @type {String}
			 */
			target->Set(NanNew("GDT_CFloat32"), NanNew(GDALGetDataTypeName(GDT_CFloat32)));
			/**
			 * Complex Float64
			 * @final
			 * @property gdal.GDT_CFloat64
			 * @type {String}
			 */
			target->Set(NanNew("GDT_CFloat64"), NanNew(GDALGetDataTypeName(GDT_CFloat64)));

			/**
			 * @class Constants (OJ)
			 */

			/**
			 * @final
			 * @property gdal.OJUndefined
			 * @type {String}
			 */
			target->Set(NanNew("OJUndefined"), NanUndefined());
			/**
			 * @final
			 * @property gdal.OJLeft
			 * @type {String}
			 */
			target->Set(NanNew("OJLeft"), NanNew("Left"));
			/**
			 * @final
			 * @property gdal.OJRight
			 * @type {String}
			 */
			target->Set(NanNew("OJRight"), NanNew("Right"));

			/**
			 * @class Constants (GCI)
			 */

			/**
			 * @final
			 * @property gdal.GCI_Undefined
			 * @type {String}
			 */
			target->Set(NanNew("GCI_Undefined"), NanUndefined());
			/**
			 * @final
			 * @property gdal.GCI_GrayIndex
			 * @type {String}
			 */
			target->Set(NanNew("GCI_GrayIndex"), NanNew(GDALGetColorInterpretationName(GCI_GrayIndex)));
			/**
			 * @final
			 * @property gdal.GCI_PaletteIndex
			 * @type {String}
			 */
			target->Set(NanNew("GCI_PaletteIndex"), NanNew(GDALGetColorInterpretationName(GCI_PaletteIndex)));
			/**
			 * @final
			 * @property gdal.GCI_RedBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_RedBand"), NanNew(GDALGetColorInterpretationName(GCI_RedBand)));
			/**
			 * @final
			 * @property gdal.GCI_GreenBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_GreenBand"), NanNew(GDALGetColorInterpretationName(GCI_GreenBand)));
			/**
			 * @final
			 * @property gdal.GCI_BlueBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_BlueBand"), NanNew(GDALGetColorInterpretationName(GCI_BlueBand)));
			/**
			 * @final
			 * @property gdal.GCI_AlphaBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_AlphaBand"), NanNew(GDALGetColorInterpretationName(GCI_AlphaBand)));
			/**
			 * @final
			 * @property gdal.GCI_HueBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_HueBand"), NanNew(GDALGetColorInterpretationName(GCI_HueBand)));
			/**
			 * @final
			 * @property gdal.GCI_SaturationBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_SaturationBand"), NanNew(GDALGetColorInterpretationName(GCI_SaturationBand)));
			/**
			 * @final
			 * @property gdal.GCI_LightnessBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_LightnessBand"), NanNew(GDALGetColorInterpretationName(GCI_LightnessBand)));
			/**
			 * @final
			 * @property gdal.GCI_CyanBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_CyanBand"), NanNew(GDALGetColorInterpretationName(GCI_CyanBand)));
			/**
			 * @final
			 * @property gdal.GCI_MagentaBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_MagentaBand"), NanNew(GDALGetColorInterpretationName(GCI_MagentaBand)));
			/**
			 * @final
			 * @property gdal.GCI_YellowBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_YellowBand"), NanNew(GDALGetColorInterpretationName(GCI_YellowBand)));
			/**
			 * @final
			 * @property gdal.GCI_BlackBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_BlackBand"), NanNew(GDALGetColorInterpretationName(GCI_BlackBand)));
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_YBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_YCbCr_YBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_YBand)));
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_CbBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_YCbCr_CbBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_CbBand)));
			/**
			 * @final
			 * @property gdal.GCI_YCbCr_CrBand
			 * @type {String}
			 */
			target->Set(NanNew("GCI_YCbCr_CrBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_CrBand)));

			/**
			 * @class Constants (wkbVariant)
			 */

			/**
			 * @final
			 * @property gdal.wkbVariantOgc
			 * @type {String}
			 */
			target->Set(NanNew("wkbVariantOgc"), NanNew("OGC"));
			/**
			 * @final
			 * @property gdal.wkbVariantIso
			 * @type {String}
			 */
			target->Set(NanNew("wkbVariantIso"), NanNew("ISO"));

			/**
			 * @class Constants (wkbByteOrder)
			 */

			/**
			 * @final
			 * @property gdal.wkbXDR
			 * @type {String}
			 */
			target->Set(NanNew("wkbXDR"), NanNew("MSB"));
			/**
			 * @final
			 * @property gdal.wkbNDR
			 * @type {String}
			 */
			target->Set(NanNew("wkbNDR"), NanNew("LSB"));

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
			target->Set(NanNew("wkb25DBit"), NanNew<Integer>(wkb25DBit));

			int wkbLinearRing25D = wkbLinearRing | wkb25DBit;

			/**
			 * @final
			 * @property gdal.wkbUnknown
			 * @type {integer}
			 */
			target->Set(NanNew("wkbUnknown"), NanNew<Integer>(wkbUnknown));
			/**
			 * @final
			 * @property gdal.wkbPoint
			 * @type {integer}
			 */
			target->Set(NanNew("wkbPoint"), NanNew<Integer>(wkbPoint));
			/**
			 * @final
			 * @property gdal.wkbLineString
			 * @type {integer}
			 */
			target->Set(NanNew("wkbLineString"), NanNew<Integer>(wkbLineString));
			/**
			 * @final
			 * @property gdal.wkbPolygon
			 * @type {integer}
			 */
			target->Set(NanNew("wkbPolygon"), NanNew<Integer>(wkbPolygon));
			/**
			 * @final
			 * @property gdal.wkbMultiPoint
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiPoint"), NanNew<Integer>(wkbMultiPoint));
			/**
			 * @final
			 * @property gdal.wkbMultiLineString
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiLineString"), NanNew<Integer>(wkbMultiLineString));
			/**
			 * @final
			 * @property gdal.wkbMultiPolygon
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiPolygon"), NanNew<Integer>(wkbMultiPolygon));
			/**
			 * @final
			 * @property gdal.wkbGeometryCollection
			 * @type {integer}
			 */
			target->Set(NanNew("wkbGeometryCollection"), NanNew<Integer>(wkbGeometryCollection));
			/**
			 * @final
			 * @property gdal.wkbNone
			 * @type {integer}
			 */
			target->Set(NanNew("wkbNone"), NanNew<Integer>(wkbNone));
			/**
			 * @final
			 * @property gdal.wkbLinearRing
			 * @type {String}
			 */
			target->Set(NanNew("wkbLinearRing"), NanNew<Integer>(wkbLinearRing));
			/**
			 * @final
			 * @property gdal.wkbPoint25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbPoint25D"), NanNew<Integer>(wkbPoint25D));
			/**
			 * @final
			 * @property gdal.wkbLineString25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbLineString25D"), NanNew<Integer>(wkbLineString25D));
			/**
			 * @final
			 * @property gdal.wkbPolygon25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbPolygon25D"), NanNew<Integer>(wkbPolygon25D));
			/**
			 * @final
			 * @property gdal.wkbMultiPoint25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiPoint25D"), NanNew<Integer>(wkbMultiPoint25D));
			/**
			 * @final
			 * @property gdal.wkbMultiLineString25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiLineString25D"), NanNew<Integer>(wkbMultiLineString25D));
			/**
			 * @final
			 * @property gdal.wkbMultiPolygon25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbMultiPolygon25D"), NanNew<Integer>(wkbMultiPolygon25D));
			/**
			 * @final
			 * @property gdal.wkbGeometryCollection25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbGeometryCollection25D"), NanNew<Integer>(wkbGeometryCollection25D));
			/**
			 * @final
			 * @property gdal.wkbLinearRing25D
			 * @type {integer}
			 */
			target->Set(NanNew("wkbLinearRing25D"), NanNew<Integer>(wkbLinearRing25D));

			/**
			 * @class Constants (OFT)
			 */

			/**
			 * @final
			 * @property gdal.OFTInteger
			 * @type {String}
			 */
			target->Set(NanNew("OFTInteger"), NanNew(getFieldTypeName(OFTInteger)));
			/**
			 * @final
			 * @property gdal.OFTIntegerList
			 * @type {String}
			 */
			target->Set(NanNew("OFTIntegerList"), NanNew(getFieldTypeName(OFTIntegerList)));
			/**
			 * @final
			 * @property gdal.OFTReal
			 * @type {String}
			 */
			target->Set(NanNew("OFTReal"), NanNew(getFieldTypeName(OFTReal)));
			/**
			 * @final
			 * @property gdal.OFTRealList
			 * @type {String}
			 */
			target->Set(NanNew("OFTRealList"), NanNew(getFieldTypeName(OFTRealList)));
			/**
			 * @final
			 * @property gdal.OFTString
			 * @type {String}
			 */
			target->Set(NanNew("OFTString"), NanNew(getFieldTypeName(OFTString)));
			/**
			 * @final
			 * @property gdal.OFTStringList
			 * @type {String}
			 */
			target->Set(NanNew("OFTStringList"), NanNew(getFieldTypeName(OFTStringList)));
			/**
			 * @final
			 * @property gdal.OFTWideString
			 * @type {String}
			 */
			target->Set(NanNew("OFTWideString"), NanNew(getFieldTypeName(OFTWideString)));
			/**
			 * @final
			 * @property gdal.OFTWideStringList
			 * @type {String}
			 */
			target->Set(NanNew("OFTWideStringList"), NanNew(getFieldTypeName(OFTWideStringList)));
			/**
			 * @final
			 * @property gdal.OFTBinary
			 * @type {String}
			 */
			target->Set(NanNew("OFTBinary"), NanNew(getFieldTypeName(OFTBinary)));
			/**
			 * @final
			 * @property gdal.OFTDate
			 * @type {String}
			 */
			target->Set(NanNew("OFTDate"), NanNew(getFieldTypeName(OFTDate)));
			/**
			 * @final
			 * @property gdal.OFTTime
			 * @type {String}
			 */
			target->Set(NanNew("OFTTime"), NanNew(getFieldTypeName(OFTTime)));
			/**
			 * @final
			 * @property gdal.OFTDateTime
			 * @type {String}
			 */
			target->Set(NanNew("OFTDateTime"), NanNew(getFieldTypeName(OFTDateTime)));

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
			target->Set(NanNew("GRA_NearestNeighbor"), NanNew("NearestNeighbor"));
			/**
			 * @final
			 * @property gdal.GRA_Bilinear
			 * @type {String}
			 */
			target->Set(NanNew("GRA_Bilinear"), NanNew("Bilinear"));
			/**
			 * @final
			 * @property gdal.GRA_Cubic
			 * @type {String}
			 */
			target->Set(NanNew("GRA_Cubic"), NanNew("Cubic"));
			/**
			 * @final
			 * @property gdal.GRA_CubicSpline
			 * @type {String}
			 */
			target->Set(NanNew("GRA_CubicSpline"), NanNew("CubicSpline"));
			/**
			 * @final
			 * @property gdal.GRA_Lanczos
			 * @type {String}
			 */
			target->Set(NanNew("GRA_Lanczos"), NanNew("Lanczos"));
			/**
			 * @final
			 * @property gdal.GRA_Average
			 * @type {String}
			 */
			target->Set(NanNew("GRA_Average"), NanNew("Average"));
			/**
			 * @final
			 * @property gdal.GRA_Mode
			 * @type {String}
			 */
			target->Set(NanNew("GRA_Mode"), NanNew("Mode"));

			/**
			 * GDAL version (not the binding version)
			 *
			 * @final
			 * @for gdal
			 * @property gdal.version
			 * @type {String}
			 */
			target->Set(NanNew("version"), NanNew(GDAL_RELEASE_NAME));

			// Local<Object> versions = NanNew<Object>();
			// versions->Set(NanNew("node"), NanNew(NODE_VERSION+1));
			// versions->Set(NanNew("v8"), NanNew(V8::GetVersion()));
			// target->Set(NanNew("versions"), versions);

			/**
			 * Disables all output.
			 *
			 * @for gdal
			 * @static
			 * @method quiet
			 */
			NODE_SET_METHOD(target, "quiet", QuietOutput);

			/**
			 * Displays extra debugging information from GDAL.
			 *
			 * @for gdal
			 * @static
			 * @method verbose
			 */
			NODE_SET_METHOD(target, "verbose", VerboseOutput);

			NODE_SET_METHOD(target, "startLogging", StartLogging);
			NODE_SET_METHOD(target, "stopLogging", StopLogging);
			NODE_SET_METHOD(target, "log", Log);

			Local<Object> supports = NanNew<Object>();
			target->Set(NanNew("supports"), supports);

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
