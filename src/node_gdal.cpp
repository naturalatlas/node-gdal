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

		static void Init(Handle<Object> target)
		{

			NODE_SET_METHOD(target, "open", open);
			NODE_SET_METHOD(target, "setConfigOption", setConfigOption);
			NODE_SET_METHOD(target, "getConfigOption", getConfigOption);
			NODE_SET_METHOD(target, "decToDMS", decToDMS);

			Warper::Initialize(target);

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

			//calls GDALRegisterAll()
			GDALDrivers::Initialize(target);
			target->Set(NanNew("drivers"), GDALDrivers::New());

			// Local<Object> versions = NanNew<Object>();
			// versions->Set(NanNew("node"), NanNew(NODE_VERSION+1));
			// versions->Set(NanNew("v8"), NanNew(V8::GetVersion()));
			// target->Set(NanNew("versions"), versions);

			NODE_SET_METHOD(target, "quiet", QuietOutput);
			NODE_SET_METHOD(target, "verbose", VerboseOutput);
			NODE_SET_METHOD(target, "startLogging", StartLogging);
			NODE_SET_METHOD(target, "stopLogging", StopLogging);

			Local<Object> supports = NanNew<Object>();
			target->Set(NanNew("supports"), supports);

			NODE_DEFINE_CONSTANT(target, CPLE_OpenFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_IllegalArg);
			NODE_DEFINE_CONSTANT(target, CPLE_NotSupported);
			NODE_DEFINE_CONSTANT(target, CPLE_AssertionFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_NoWriteAccess);
			NODE_DEFINE_CONSTANT(target, CPLE_UserInterrupt);

			target->Set(NanNew("DMD_LONGNAME"), NanNew(GDAL_DMD_LONGNAME));
			target->Set(NanNew("DMD_MIMETYPE"), NanNew(GDAL_DMD_MIMETYPE));
			target->Set(NanNew("DMD_HELPTOPIC"), NanNew(GDAL_DMD_HELPTOPIC));
			target->Set(NanNew("DMD_EXTENSION"), NanNew(GDAL_DMD_EXTENSION));
			target->Set(NanNew("DMD_CREATIONOPTIONLIST"), NanNew(GDAL_DMD_CREATIONOPTIONLIST));
			target->Set(NanNew("DMD_CREATIONDATATYPES"), NanNew(GDAL_DMD_CREATIONDATATYPES));

			target->Set(NanNew("DCAP_CREATE"), NanNew(GDAL_DCAP_CREATE));
			target->Set(NanNew("DCAP_CREATECOPY"), NanNew(GDAL_DCAP_CREATECOPY));
			target->Set(NanNew("DCAP_VIRTUALIO"), NanNew(GDAL_DCAP_VIRTUALIO));

			target->Set(NanNew("OLCRandomRead"), NanNew(OLCRandomRead));
			target->Set(NanNew("OLCSequentialWrite"), NanNew(OLCSequentialWrite));
			target->Set(NanNew("OLCRandomWrite"), NanNew(OLCRandomWrite));
			target->Set(NanNew("OLCFastSpatialFilter"), NanNew(OLCFastSpatialFilter));
			target->Set(NanNew("OLCFastFeatureCount"), NanNew(OLCFastFeatureCount));
			target->Set(NanNew("OLCFastGetExtent"), NanNew(OLCFastGetExtent));
			target->Set(NanNew("OLCCreateField"), NanNew(OLCCreateField));
			target->Set(NanNew("OLCDeleteField"), NanNew(OLCDeleteField));
			target->Set(NanNew("OLCReorderFields"), NanNew(OLCReorderFields));
			target->Set(NanNew("OLCAlterFieldDefn"), NanNew(OLCAlterFieldDefn));
			target->Set(NanNew("OLCTransactions"), NanNew(OLCTransactions));
			target->Set(NanNew("OLCDeleteFeature"), NanNew(OLCDeleteFeature));
			target->Set(NanNew("OLCFastSetNextByIndex"), NanNew(OLCFastSetNextByIndex));
			target->Set(NanNew("OLCStringsAsUTF8"), NanNew(OLCStringsAsUTF8));
			target->Set(NanNew("OLCIgnoreFields"), NanNew(OLCIgnoreFields));

			#ifdef OLCCreateGeomField
			target->Set(NanNew("OLCCreateGeomField"), NanNew(OLCCreateGeomField));
			#endif
			#ifdef ODsCCreateGeomFieldAfterCreateLayer
			target->Set(NanNew("ODsCCreateGeomFieldAfterCreateLayer"), NanNew(ODsCCreateGeomFieldAfterCreateLayer));
			#endif

			target->Set(NanNew("ODsCCreateLayer"), NanNew(ODsCCreateLayer));
			target->Set(NanNew("ODsCDeleteLayer"), NanNew(ODsCDeleteLayer));
			target->Set(NanNew("ODrCCreateDataSource"), NanNew(ODrCCreateDataSource));
			target->Set(NanNew("ODrCDeleteDataSource"), NanNew(ODrCDeleteDataSource));


			NODE_DEFINE_CONSTANT(target, GA_ReadOnly);
			NODE_DEFINE_CONSTANT(target, GA_Update);
			NODE_DEFINE_CONSTANT(target, GF_Read);
			NODE_DEFINE_CONSTANT(target, GF_Write);

			target->Set(NanNew("GDT_Unknown"), NanUndefined());
			target->Set(NanNew("GDT_Byte"), NanNew(GDALGetDataTypeName(GDT_Byte)));
			target->Set(NanNew("GDT_UInt16"), NanNew(GDALGetDataTypeName(GDT_UInt16)));
			target->Set(NanNew("GDT_Int16"), NanNew(GDALGetDataTypeName(GDT_Int16)));
			target->Set(NanNew("GDT_UInt32"), NanNew(GDALGetDataTypeName(GDT_UInt32)));
			target->Set(NanNew("GDT_Int32"), NanNew(GDALGetDataTypeName(GDT_Int32)));
			target->Set(NanNew("GDT_Float32"), NanNew(GDALGetDataTypeName(GDT_Float32)));
			target->Set(NanNew("GDT_Float64"), NanNew(GDALGetDataTypeName(GDT_Float64)));
			target->Set(NanNew("GDT_CInt16"), NanNew(GDALGetDataTypeName(GDT_CInt16)));
			target->Set(NanNew("GDT_CInt32"), NanNew(GDALGetDataTypeName(GDT_CInt32)));
			target->Set(NanNew("GDT_CFloat32"), NanNew(GDALGetDataTypeName(GDT_CFloat32)));
			target->Set(NanNew("GDT_CFloat64"), NanNew(GDALGetDataTypeName(GDT_CFloat64)));

			target->Set(NanNew("OJUndefined"), NanUndefined());
			target->Set(NanNew("OJLeft"), NanNew("Left"));
			target->Set(NanNew("OJRight"), NanNew("Right"));

			target->Set(NanNew("GCI_Undefined"), NanUndefined());
			target->Set(NanNew("GCI_GrayIndex"), NanNew(GDALGetColorInterpretationName(GCI_GrayIndex)));
			target->Set(NanNew("GCI_PaletteIndex"), NanNew(GDALGetColorInterpretationName(GCI_PaletteIndex)));
			target->Set(NanNew("GCI_RedBand"), NanNew(GDALGetColorInterpretationName(GCI_RedBand)));
			target->Set(NanNew("GCI_GreenBand"), NanNew(GDALGetColorInterpretationName(GCI_GreenBand)));
			target->Set(NanNew("GCI_BlueBand"), NanNew(GDALGetColorInterpretationName(GCI_BlueBand)));
			target->Set(NanNew("GCI_AlphaBand"), NanNew(GDALGetColorInterpretationName(GCI_AlphaBand)));
			target->Set(NanNew("GCI_HueBand"), NanNew(GDALGetColorInterpretationName(GCI_HueBand)));
			target->Set(NanNew("GCI_SaturationBand"), NanNew(GDALGetColorInterpretationName(GCI_SaturationBand)));
			target->Set(NanNew("GCI_LightnessBand"), NanNew(GDALGetColorInterpretationName(GCI_LightnessBand)));
			target->Set(NanNew("GCI_CyanBand"), NanNew(GDALGetColorInterpretationName(GCI_CyanBand)));
			target->Set(NanNew("GCI_MagentaBand"), NanNew(GDALGetColorInterpretationName(GCI_MagentaBand)));
			target->Set(NanNew("GCI_YellowBand"), NanNew(GDALGetColorInterpretationName(GCI_YellowBand)));
			target->Set(NanNew("GCI_BlackBand"), NanNew(GDALGetColorInterpretationName(GCI_BlackBand)));
			target->Set(NanNew("GCI_YCbCr_YBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_YBand)));
			target->Set(NanNew("GCI_YCbCr_CbBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_CbBand)));
			target->Set(NanNew("GCI_YCbCr_CrBand"), NanNew(GDALGetColorInterpretationName(GCI_YCbCr_CrBand)));

			target->Set(NanNew("wkbVariantOgc"), NanNew("OGC"));
			target->Set(NanNew("wkbVariantIso"), NanNew("ISO"));
			target->Set(NanNew("wkbXDR"), NanNew("MSB"));
			target->Set(NanNew("wkbNDR"), NanNew("LSB"));

			target->Set(NanNew("wkb25DBit"), NanNew<Integer>(wkb25DBit));

			int wkbLinearRing25D = wkbLinearRing | wkb25DBit;

			target->Set(NanNew("wkbUnknown"), NanNew<Integer>(wkbUnknown));
			target->Set(NanNew("wkbPoint"), NanNew<Integer>(wkbPoint));
			target->Set(NanNew("wkbLineString"), NanNew<Integer>(wkbLineString));
			target->Set(NanNew("wkbPolygon"), NanNew<Integer>(wkbPolygon));
			target->Set(NanNew("wkbMultiPoint"), NanNew<Integer>(wkbMultiPoint));
			target->Set(NanNew("wkbMultiLineString"), NanNew<Integer>(wkbMultiLineString));
			target->Set(NanNew("wkbMultiPolygon"), NanNew<Integer>(wkbMultiPolygon));
			target->Set(NanNew("wkbGeometryCollection"), NanNew<Integer>(wkbGeometryCollection));
			target->Set(NanNew("wkbNone"), NanNew<Integer>(wkbNone));
			target->Set(NanNew("wkbLinearRing"), NanNew<Integer>(wkbLinearRing));
			target->Set(NanNew("wkbPoint25D"), NanNew<Integer>(wkbPoint25D));
			target->Set(NanNew("wkbLineString25D"), NanNew<Integer>(wkbLineString25D));
			target->Set(NanNew("wkbPolygon25D"), NanNew<Integer>(wkbPolygon25D));
			target->Set(NanNew("wkbMultiPoint25D"), NanNew<Integer>(wkbMultiPoint25D));
			target->Set(NanNew("wkbMultiLineString25D"), NanNew<Integer>(wkbMultiLineString25D));
			target->Set(NanNew("wkbMultiPolygon25D"), NanNew<Integer>(wkbMultiPolygon25D));
			target->Set(NanNew("wkbGeometryCollection25D"), NanNew<Integer>(wkbGeometryCollection25D));
			target->Set(NanNew("wkbLinearRing25D"), NanNew<Integer>(wkbLinearRing25D));

			target->Set(NanNew("OFTInteger"), NanNew(getFieldTypeName(OFTInteger)));
			target->Set(NanNew("OFTIntegerList"), NanNew(getFieldTypeName(OFTIntegerList)));
			target->Set(NanNew("OFTReal"), NanNew(getFieldTypeName(OFTReal)));
			target->Set(NanNew("OFTRealList"), NanNew(getFieldTypeName(OFTRealList)));
			target->Set(NanNew("OFTString"), NanNew(getFieldTypeName(OFTString)));
			target->Set(NanNew("OFTStringList"), NanNew(getFieldTypeName(OFTStringList)));
			target->Set(NanNew("OFTWideString"), NanNew(getFieldTypeName(OFTWideString)));
			target->Set(NanNew("OFTWideStringList"), NanNew(getFieldTypeName(OFTWideStringList)));
			target->Set(NanNew("OFTBinary"), NanNew(getFieldTypeName(OFTBinary)));
			target->Set(NanNew("OFTDate"), NanNew(getFieldTypeName(OFTDate)));
			target->Set(NanNew("OFTTime"), NanNew(getFieldTypeName(OFTTime)));
			target->Set(NanNew("OFTDateTime"), NanNew(getFieldTypeName(OFTDateTime)));

			target->Set(NanNew("GRA_NearestNeighbor"), NanNew("NearestNeighbor"));
			target->Set(NanNew("GRA_Bilinear"), NanNew("Bilinear"));
			target->Set(NanNew("GRA_Cubic"), NanNew("Cubic"));
			target->Set(NanNew("GRA_CubicSpline"), NanNew("CubicSpline"));
			target->Set(NanNew("GRA_Lanczos"), NanNew("Lanczos"));
			target->Set(NanNew("GRA_Average"), NanNew("Average"));
			target->Set(NanNew("GRA_Mode"), NanNew("Mode"));

			target->Set(NanNew("version"), NanNew(GDAL_RELEASE_NAME));

			target->Set(NanNew("CreateDataSourceOption"), NanNew(ODrCCreateDataSource));
			target->Set(NanNew("DeleteDataSourceOption"), NanNew(ODrCDeleteDataSource));
		}

	}

} // namespace node_gdal

NODE_MODULE(gdal, node_gdal::Init)
