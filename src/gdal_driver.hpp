#ifndef __NODE_GDAL_DRIVER_H__
#define __NODE_GDAL_DRIVER_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal_priv.h>

// ogr
#include <ogrsf_frmts.h>

#include "utils/obj_cache.hpp"

using namespace v8;
using namespace node;

// > GDAL 2.0 : a wrapper for GDALDriver
// < GDAL 2.0 : a wrapper for either a GDALDriver or OGRSFDriver that behaves like a 2.0 Driver
// 
namespace node_gdal {

class Driver: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(GDALDriver *driver);
	static NAN_METHOD(toString);
	static NAN_METHOD(open);
	static NAN_METHOD(create);
	static NAN_METHOD(createCopy);
	static NAN_METHOD(deleteDataset);
	static NAN_METHOD(rename);
	static NAN_METHOD(copyFiles);
	static NAN_METHOD(getMetadata);

	static ObjectCache<GDALDriver, Driver>  cache;

	static NAN_GETTER(descriptionGetter);

	Driver();
	Driver(GDALDriver *driver);
	inline GDALDriver *getGDALDriver() {
		return this_gdaldriver;
	}
	void dispose();

	#if GDAL_VERSION_MAJOR < 2
	static Local<Value> New(OGRSFDriver *driver);

	static ObjectCache<OGRSFDriver, Driver> cache_ogr;

	Driver(OGRSFDriver *driver);
	
	inline OGRSFDriver *getOGRSFDriver() {
		return this_ogrdriver;
	}
	inline bool isAlive(){
		return this_ogrdriver || this_gdaldriver;
	}
	bool uses_ogr;
	#else
	inline bool isAlive(){
		return this_gdaldriver;
	}
	#endif

private:
	~Driver();
	GDALDriver *this_gdaldriver;
	#if GDAL_VERSION_MAJOR < 2
	OGRSFDriver *this_ogrdriver;
	#endif
};

}
#endif
