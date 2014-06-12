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
#include "gdal.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"

// node-ogr
#include "ogr.hpp"

//collections
#include "collections/rasterband.hpp"
#include "collections/layer.hpp"
#include "collections/feature.hpp"

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
			CPLSetErrorHandler(NULL);
			return Undefined();
		}

		static void Init(Handle<Object> target)
		{

			NODE_SET_METHOD(target, "open", node_gdal::open);
			NODE_SET_METHOD(target, "openShared", node_gdal::openShared);
			NODE_SET_METHOD(target, "getDriverByName", node_gdal::getDriverByName);
			NODE_SET_METHOD(target, "getDriverCount", node_gdal::getDriverCount);
			NODE_SET_METHOD(target, "getDriver", node_gdal::getDriver);
			NODE_SET_METHOD(target, "close", node_gdal::close);

			MajorObject::Initialize(target);
			Driver::Initialize(target);
			Dataset::Initialize(target);
			RasterBand::Initialize(target);

			RasterBandCollection::Initialize(target);
			LayerCollection::Initialize(target);
			FeatureCollection::Initialize(target);

			Local<Object> versions = Object::New();
			versions->Set(String::NewSymbol("node"), String::New(NODE_VERSION+1));
			versions->Set(String::NewSymbol("v8"), String::New(V8::GetVersion()));

			target->Set(String::NewSymbol("versions"), versions);

			NODE_SET_METHOD(target, "quiet", QuietOutput);
			NODE_SET_METHOD(target, "verbose", VerboseOutput);

			Local<Object> supports = Object::New();
			target->Set(String::NewSymbol("supports"), supports);

			GDALAllRegister();

			//by default don't print errors to stderr
			CPLSetErrorHandler(CPLQuietErrorHandler);

			GDALDriverManager  *reg = GetGDALDriverManager();

			int driver_count = reg->GetDriverCount();

			Local<Array> supported_drivers = Array::New(driver_count);

			for (int i = 0; i < driver_count; ++i) {
				GDALDriver *driver = reg->GetDriver(i);
				supported_drivers->Set(Integer::New(static_cast<int>(i)), String::New(driver->GetDescription()));
			}

			target->Set(String::NewSymbol("drivers"), supported_drivers);

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

			NODE_DEFINE_CONSTANT(target, GDT_Unknown);
			NODE_DEFINE_CONSTANT(target, GDT_Byte);
			NODE_DEFINE_CONSTANT(target, GDT_UInt16);
			NODE_DEFINE_CONSTANT(target, GDT_Int16);
			NODE_DEFINE_CONSTANT(target, GDT_UInt32);
			NODE_DEFINE_CONSTANT(target, GDT_Int32);
			NODE_DEFINE_CONSTANT(target, GDT_Float32);
			NODE_DEFINE_CONSTANT(target, GDT_Float64);
			NODE_DEFINE_CONSTANT(target, GDT_CInt16);
			NODE_DEFINE_CONSTANT(target, GDT_CInt32);
			NODE_DEFINE_CONSTANT(target, GDT_CFloat32);
			NODE_DEFINE_CONSTANT(target, GDT_CFloat64);


			NODE_DEFINE_CONSTANT(target, GCI_Undefined);
			NODE_DEFINE_CONSTANT(target, GCI_GrayIndex);
			NODE_DEFINE_CONSTANT(target, GCI_PaletteIndex);
			NODE_DEFINE_CONSTANT(target, GCI_RedBand);
			NODE_DEFINE_CONSTANT(target, GCI_GreenBand);
			NODE_DEFINE_CONSTANT(target, GCI_BlueBand);
			NODE_DEFINE_CONSTANT(target, GCI_AlphaBand);
			NODE_DEFINE_CONSTANT(target, GCI_HueBand);
			NODE_DEFINE_CONSTANT(target, GCI_SaturationBand);
			NODE_DEFINE_CONSTANT(target, GCI_LightnessBand);
			NODE_DEFINE_CONSTANT(target, GCI_CyanBand);
			NODE_DEFINE_CONSTANT(target, GCI_MagentaBand);
			NODE_DEFINE_CONSTANT(target, GCI_YellowBand);
			NODE_DEFINE_CONSTANT(target, GCI_BlackBand);
			NODE_DEFINE_CONSTANT(target, GCI_YCbCr_YBand);
			NODE_DEFINE_CONSTANT(target, GCI_YCbCr_CbBand);
			NODE_DEFINE_CONSTANT(target, GCI_YCbCr_CrBand);
			NODE_DEFINE_CONSTANT(target, GCI_Max);

			Local<Object> ogr = Object::New();
			target->Set(String::NewSymbol("ogr"), ogr);
			::node_ogr::Init(ogr);
		}

	}

} // namespace node_gdal

NODE_MODULE(gdal, node_gdal::Init)
