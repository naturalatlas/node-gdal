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

#include "obj_cache.hpp"

using namespace v8;
using namespace node;

// > GDAL 2.0 : a wrapper for GDALDataset
// < GDAL 2.0 : a wrapper for either a GDALDataset or OGRDataSource that behaves like a 2.0 Dataset

namespace node_gdal {

class Dataset: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	static Handle<Value> New(GDALDataset *ds);
	static Handle<Value> New(OGRDataSource *ds);
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

	static NAN_SETTER(srsSetter);
	static NAN_SETTER(geoTransformSetter);

	static ObjectCache<GDALDataset>   dataset_cache;
	static ObjectCache<OGRDataSource> datasource_cache;

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

}
#endif
