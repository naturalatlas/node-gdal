#ifndef __NODE_GDAL_DRIVER_H__
#define __NODE_GDAL_DRIVER_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

// ogr
#include <ogrsf_frmts.h>

#include "obj_cache.hpp"

using namespace v8;
using namespace node;

// > GDAL 2.0 : a wrapper for GDALDriver
// < GDAL 2.0 : a wrapper for either a GDALDriver or OGRSFDriver that behaves like a 2.0 Driver
// 
class Driver: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALDriver *driver);
	static Handle<Value> New(OGRSFDriver *driver);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> open(const Arguments &args);
	static Handle<Value> create(const Arguments &args);
	static Handle<Value> createCopy(const Arguments &args);
	static Handle<Value> deleteDataset(const Arguments &args);
	static Handle<Value> rename(const Arguments &args);
	static Handle<Value> copyFiles(const Arguments &args);
	static Handle<Value> getMetadata(const Arguments &args);

	static ObjectCache<GDALDriver*>  cache;
	static ObjectCache<OGRSFDriver*> cache_ogr;

	static Handle<Value> descriptionGetter(Local<String> property, const AccessorInfo& info);

	Driver();
	Driver(GDALDriver *driver);
	Driver(OGRSFDriver *driver);
	inline GDALDriver *getGDALDriver() {
		return this_gdaldriver;
	}
	inline OGRSFDriver *getOGRSFDriver() {
		return this_ogrdriver;
	}

	bool uses_ogr;
private:
	~Driver();
	GDALDriver *this_gdaldriver;
	OGRSFDriver *this_ogrdriver;
};

#endif
