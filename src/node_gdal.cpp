// v8
#include <v8.h>

// node
#include <node.h>
#include <node_buffer.h>
#include <node_version.h>

// gdal
#include <gdal.h>

// node-gdal
#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"

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
#include "field_types.hpp"

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
#include "collections/gdal_drivers.hpp"

// std
#include <string>
#include <sstream>
#include <vector>

namespace node_gdal {

	using namespace node;
	using namespace v8;

	extern "C" {
		static Handle<Value> QuietOutput(const Arguments &args)
		{
			CPLSetErrorHandler(CPLQuietErrorHandler);
			return Undefined();
		}

		static Handle<Value> VerboseOutput(const Arguments &args)
		{
			CPLSetErrorHandler(CPLDefaultErrorHandler);
			return Undefined();
		}

		static void Init(Handle<Object> target)
		{

			NODE_SET_METHOD(target, "open", open);
			NODE_SET_METHOD(target, "setConfigOption", setConfigOption);
			NODE_SET_METHOD(target, "getConfigOption", getConfigOption);

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

			//calls GDALRegisterAll()
			GDALDrivers::Initialize(target);
			target->Set(String::NewSymbol("drivers"), GDALDrivers::New());

			// Local<Object> versions = Object::New();
			// versions->Set(String::NewSymbol("node"), String::New(NODE_VERSION+1));
			// versions->Set(String::NewSymbol("v8"), String::New(V8::GetVersion()));
			// target->Set(String::NewSymbol("versions"), versions);

			NODE_SET_METHOD(target, "quiet", QuietOutput);
			NODE_SET_METHOD(target, "verbose", VerboseOutput);

			Local<Object> supports = Object::New();
			target->Set(String::NewSymbol("supports"), supports);

			NODE_DEFINE_CONSTANT(target, CPLE_OpenFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_IllegalArg);
			NODE_DEFINE_CONSTANT(target, CPLE_NotSupported);
			NODE_DEFINE_CONSTANT(target, CPLE_AssertionFailed);
			NODE_DEFINE_CONSTANT(target, CPLE_NoWriteAccess);
			NODE_DEFINE_CONSTANT(target, CPLE_UserInterrupt);

			target->Set(String::NewSymbol("DMD_LONGNAME"), String::New(GDAL_DMD_LONGNAME));
			target->Set(String::NewSymbol("DMD_MIMETYPE"), String::New(GDAL_DMD_MIMETYPE));
			target->Set(String::NewSymbol("DMD_HELPTOPIC"), String::New(GDAL_DMD_HELPTOPIC));
			target->Set(String::NewSymbol("DMD_EXTENSION"), String::New(GDAL_DMD_EXTENSION));
			target->Set(String::NewSymbol("DMD_CREATIONOPTIONLIST"), String::New(GDAL_DMD_CREATIONOPTIONLIST));
			target->Set(String::NewSymbol("DMD_CREATIONDATATYPES"), String::New(GDAL_DMD_CREATIONDATATYPES));

			target->Set(String::NewSymbol("DCAP_CREATE"), String::New(GDAL_DCAP_CREATE));
			target->Set(String::NewSymbol("DCAP_CREATECOPY"), String::New(GDAL_DCAP_CREATECOPY));
			target->Set(String::NewSymbol("DCAP_VIRTUALIO"), String::New(GDAL_DCAP_VIRTUALIO));

			NODE_DEFINE_CONSTANT(target, GA_ReadOnly);
			NODE_DEFINE_CONSTANT(target, GA_Update);
			NODE_DEFINE_CONSTANT(target, GF_Read);
			NODE_DEFINE_CONSTANT(target, GF_Write);

			target->Set(String::NewSymbol("GCI_Undefined"), Undefined());
			target->Set(String::NewSymbol("GCI_GrayIndex"), String::New(GDALGetColorInterpretationName(GCI_GrayIndex)));
			target->Set(String::NewSymbol("GCI_PaletteIndex"), String::New(GDALGetColorInterpretationName(GCI_PaletteIndex)));
			target->Set(String::NewSymbol("GCI_RedBand"), String::New(GDALGetColorInterpretationName(GCI_RedBand)));
			target->Set(String::NewSymbol("GCI_GreenBand"), String::New(GDALGetColorInterpretationName(GCI_GreenBand)));
			target->Set(String::NewSymbol("GCI_BlueBand"), String::New(GDALGetColorInterpretationName(GCI_BlueBand)));
			target->Set(String::NewSymbol("GCI_AlphaBand"), String::New(GDALGetColorInterpretationName(GCI_AlphaBand)));
			target->Set(String::NewSymbol("GCI_HueBand"), String::New(GDALGetColorInterpretationName(GCI_HueBand)));
			target->Set(String::NewSymbol("GCI_SaturationBand"), String::New(GDALGetColorInterpretationName(GCI_SaturationBand)));
			target->Set(String::NewSymbol("GCI_LightnessBand"), String::New(GDALGetColorInterpretationName(GCI_LightnessBand)));
			target->Set(String::NewSymbol("GCI_CyanBand"), String::New(GDALGetColorInterpretationName(GCI_CyanBand)));
			target->Set(String::NewSymbol("GCI_MagentaBand"), String::New(GDALGetColorInterpretationName(GCI_MagentaBand)));
			target->Set(String::NewSymbol("GCI_YellowBand"), String::New(GDALGetColorInterpretationName(GCI_YellowBand)));
			target->Set(String::NewSymbol("GCI_BlackBand"), String::New(GDALGetColorInterpretationName(GCI_BlackBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_YBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_YBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_CbBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_CbBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_CrBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_CrBand)));
			
			target->Set(String::NewSymbol("GDT_Unknown"), Undefined());
			target->Set(String::NewSymbol("GDT_Byte"), String::New(GDALGetDataTypeName(GDT_Byte)));
			target->Set(String::NewSymbol("GDT_UInt16"), String::New(GDALGetDataTypeName(GDT_UInt16)));
			target->Set(String::NewSymbol("GDT_Int16"), String::New(GDALGetDataTypeName(GDT_Int16)));
			target->Set(String::NewSymbol("GDT_UInt32"), String::New(GDALGetDataTypeName(GDT_UInt32)));
			target->Set(String::NewSymbol("GDT_Int32"), String::New(GDALGetDataTypeName(GDT_Int32)));
			target->Set(String::NewSymbol("GDT_Float32"), String::New(GDALGetDataTypeName(GDT_Float32)));
			target->Set(String::NewSymbol("GDT_Float64"), String::New(GDALGetDataTypeName(GDT_Float64)));
			target->Set(String::NewSymbol("GDT_CInt16"), String::New(GDALGetDataTypeName(GDT_CInt16)));
			target->Set(String::NewSymbol("GDT_CInt32"), String::New(GDALGetDataTypeName(GDT_CInt32)));
			target->Set(String::NewSymbol("GDT_CFloat32"), String::New(GDALGetDataTypeName(GDT_CFloat32)));
			target->Set(String::NewSymbol("GDT_CFloat64"), String::New(GDALGetDataTypeName(GDT_CFloat64)));

			target->Set(String::NewSymbol("OJUndefined"), Undefined());
			target->Set(String::NewSymbol("OJLeft"), String::New("Left"));
			target->Set(String::NewSymbol("OJRight"), String::New("Right"));

			target->Set(String::NewSymbol("GCI_Undefined"), Undefined());
			target->Set(String::NewSymbol("GCI_GrayIndex"), String::New(GDALGetColorInterpretationName(GCI_GrayIndex)));
			target->Set(String::NewSymbol("GCI_PaletteIndex"), String::New(GDALGetColorInterpretationName(GCI_PaletteIndex)));
			target->Set(String::NewSymbol("GCI_RedBand"), String::New(GDALGetColorInterpretationName(GCI_RedBand)));
			target->Set(String::NewSymbol("GCI_GreenBand"), String::New(GDALGetColorInterpretationName(GCI_GreenBand)));
			target->Set(String::NewSymbol("GCI_BlueBand"), String::New(GDALGetColorInterpretationName(GCI_BlueBand)));
			target->Set(String::NewSymbol("GCI_AlphaBand"), String::New(GDALGetColorInterpretationName(GCI_AlphaBand)));
			target->Set(String::NewSymbol("GCI_HueBand"), String::New(GDALGetColorInterpretationName(GCI_HueBand)));
			target->Set(String::NewSymbol("GCI_SaturationBand"), String::New(GDALGetColorInterpretationName(GCI_SaturationBand)));
			target->Set(String::NewSymbol("GCI_LightnessBand"), String::New(GDALGetColorInterpretationName(GCI_LightnessBand)));
			target->Set(String::NewSymbol("GCI_CyanBand"), String::New(GDALGetColorInterpretationName(GCI_CyanBand)));
			target->Set(String::NewSymbol("GCI_MagentaBand"), String::New(GDALGetColorInterpretationName(GCI_MagentaBand)));
			target->Set(String::NewSymbol("GCI_YellowBand"), String::New(GDALGetColorInterpretationName(GCI_YellowBand)));
			target->Set(String::NewSymbol("GCI_BlackBand"), String::New(GDALGetColorInterpretationName(GCI_BlackBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_YBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_YBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_CbBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_CbBand)));
			target->Set(String::NewSymbol("GCI_YCbCr_CrBand"), String::New(GDALGetColorInterpretationName(GCI_YCbCr_CrBand)));
			
			NODE_DEFINE_CONSTANT(target, wkbUnknown);
			NODE_DEFINE_CONSTANT(target, wkbPoint);
			NODE_DEFINE_CONSTANT(target, wkbLineString);
			NODE_DEFINE_CONSTANT(target, wkbPolygon);
			NODE_DEFINE_CONSTANT(target, wkbMultiPoint);
			NODE_DEFINE_CONSTANT(target, wkbMultiLineString);
			NODE_DEFINE_CONSTANT(target, wkbMultiPolygon);
			NODE_DEFINE_CONSTANT(target, wkbGeometryCollection);
			NODE_DEFINE_CONSTANT(target, wkbNone);
			NODE_DEFINE_CONSTANT(target, wkbLinearRing);
			NODE_DEFINE_CONSTANT(target, wkbPoint25D);
			NODE_DEFINE_CONSTANT(target, wkbLineString25D);
			NODE_DEFINE_CONSTANT(target, wkbPolygon25D);
			NODE_DEFINE_CONSTANT(target, wkbMultiPoint25D);
			NODE_DEFINE_CONSTANT(target, wkbMultiLineString25D);
			NODE_DEFINE_CONSTANT(target, wkbMultiPolygon25D);
			NODE_DEFINE_CONSTANT(target, wkbGeometryCollection25D);

			target->Set(String::NewSymbol("OFTInteger"), String::New(getFieldTypeName(OFTInteger)));
			target->Set(String::NewSymbol("OFTIntegerList"), String::New(getFieldTypeName(OFTIntegerList)));
			target->Set(String::NewSymbol("OFTReal"), String::New(getFieldTypeName(OFTReal)));
			target->Set(String::NewSymbol("OFTRealList"), String::New(getFieldTypeName(OFTRealList)));
			target->Set(String::NewSymbol("OFTString"), String::New(getFieldTypeName(OFTString)));
			target->Set(String::NewSymbol("OFTStringList"), String::New(getFieldTypeName(OFTStringList)));
			target->Set(String::NewSymbol("OFTWideString"), String::New(getFieldTypeName(OFTWideString)));
			target->Set(String::NewSymbol("OFTWideStringList"), String::New(getFieldTypeName(OFTWideStringList)));
			target->Set(String::NewSymbol("OFTBinary"), String::New(getFieldTypeName(OFTBinary)));
			target->Set(String::NewSymbol("OFTDate"), String::New(getFieldTypeName(OFTDate)));
			target->Set(String::NewSymbol("OFTTime"), String::New(getFieldTypeName(OFTTime)));
			target->Set(String::NewSymbol("OFTDateTime"), String::New(getFieldTypeName(OFTDateTime)));

			target->Set(String::NewSymbol("version"), String::New(GDAL_RELEASE_NAME));

			target->Set(String::NewSymbol("CreateDataSourceOption"), String::New(ODrCCreateDataSource));
			target->Set(String::NewSymbol("DeleteDataSourceOption"), String::New(ODrCDeleteDataSource));
		}

	}

} // namespace node_gdal

NODE_MODULE(gdal, node_gdal::Init)
