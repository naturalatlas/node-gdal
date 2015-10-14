#ifndef __NODE_GDAL_DATASET_H__
#define __NODE_GDAL_DATASET_H__

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

// > GDAL 2.0 : a wrapper for GDALDataset
// < GDAL 2.0 : a wrapper for either a GDALDataset or OGRDataSource that behaves like a 2.0 Dataset

namespace node_gdal {

class Dataset: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(GDALDataset *ds);
	static NAN_METHOD(toString);
	static NAN_METHOD(flush);
	static NAN_METHOD(getMetadata);
	static NAN_METHOD(getFileList);
	static NAN_METHOD(getGCPProjection);
	static NAN_METHOD(getGCPs);
	static NAN_METHOD(setGCPs);
	static NAN_METHOD(executeSQL);
	static NAN_METHOD(testCapability);
	static NAN_METHOD(buildOverviews);
	static NAN_METHOD(close);

	static NAN_GETTER(bandsGetter);
	static NAN_GETTER(rasterSizeGetter);
	static NAN_GETTER(srsGetter);
	static NAN_GETTER(driverGetter);
	static NAN_GETTER(geoTransformGetter);
	static NAN_GETTER(descriptionGetter);
	static NAN_GETTER(layersGetter);
	static NAN_GETTER(uidGetter);

	static NAN_SETTER(srsSetter);
	static NAN_SETTER(geoTransformSetter);

	static ObjectCache<GDALDataset, Dataset> dataset_cache;

	Dataset(GDALDataset *ds);
	inline GDALDataset *getDataset() {
		return this_dataset;
	}

	void dispose();
    long uid;

	#if GDAL_VERSION_MAJOR < 2
	static Local<Value> New(OGRDataSource *ds);
	static ObjectCache<OGRDataSource, Dataset> datasource_cache;
	Dataset(OGRDataSource *ds);
	inline OGRDataSource *getDatasource() {
		return this_datasource;
	}
	inline bool isAlive(){
		return (uses_ogr ? (this_datasource != NULL) : (this_dataset != NULL) ) && ptr_manager.isAlive(uid);
	}
	bool uses_ogr;
	#else
	inline bool isAlive(){
		return this_dataset && ptr_manager.isAlive(uid);
	}
	#endif

private:
	~Dataset();
	GDALDataset   *this_dataset;
	#if GDAL_VERSION_MAJOR < 2
	OGRDataSource *this_datasource;
	#endif
};

}
#endif
