#ifndef __NODE_GDAL_DATASET_H__
#define __NODE_GDAL_DATASET_H__

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

// > GDAL 2.0 : a wrapper for GDALDataset
// < GDAL 2.0 : a wrapper for either a GDALDataset or OGRDataSource that behaves like a 2.0 Dataset

class Dataset: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALDataset *ds);
	static Handle<Value> New(OGRDataSource *ds);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> flush(const Arguments &args);
	static Handle<Value> getMetadata(const Arguments &args);
	static Handle<Value> getFileList(const Arguments &args);
	static Handle<Value> getGCPProjection(const Arguments &args);
	static Handle<Value> getGCPs(const Arguments &args);
	static Handle<Value> setGCPs(const Arguments &args);
	static Handle<Value> executeSQL(const Arguments &args);
	static Handle<Value> testCapability(const Arguments &args);
	static Handle<Value> close(const Arguments &args);

	static Handle<Value> bandsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> rasterSizeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> srsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> driverGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> geoTransformGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> descriptionGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> layersGetter(Local<String> property, const AccessorInfo &info);

	static void srsSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void geoTransformSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);

	static ObjectCache<GDALDataset*>   dataset_cache;
	static ObjectCache<OGRDataSource*> datasource_cache;

	Dataset(GDALDataset *ds);
	Dataset(OGRDataSource *ds);
	inline GDALDataset *getDataset() {
		return this_dataset;
	}
	inline OGRDataSource *getDatasource() {
		return this_datasource;
	}
	void dispose();

	bool uses_ogr;
private:
	~Dataset();
	GDALDataset   *this_dataset;
	OGRDataSource *this_datasource;
	std::vector<OGRLayer*> result_sets;
};

#endif
