#ifndef __NODE_OGR_LAYER_H__
#define __NODE_OGR_LAYER_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// ogr
#include <ogrsf_frmts.h>

#include "utils/obj_cache.hpp"
#include "gdal_dataset.hpp"

using namespace v8;
using namespace node;

namespace node_gdal {

class Layer: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	#if GDAL_VERSION_MAJOR >= 2
	static Handle<Value> New(OGRLayer *raw, GDALDataset *raw_parent);
	static Handle<Value> New(OGRLayer *raw, GDALDataset *raw_parent, bool result_set);
	#else
	static Handle<Value> New(OGRLayer *raw, OGRDataSource *raw_parent);
	static Handle<Value> New(OGRLayer *raw, OGRDataSource *raw_parent, bool result_set);
	#endif
	static NAN_METHOD(toString);
	static NAN_METHOD(getExtent);
	static NAN_METHOD(setAttributeFilter);
	static NAN_METHOD(setSpatialFilter);
	static NAN_METHOD(getSpatialFilter);
	static NAN_METHOD(testCapability);
	static NAN_METHOD(syncToDisk);

	static NAN_SETTER(dsSetter);
	static NAN_GETTER(dsGetter);
	static NAN_GETTER(srsGetter);
	static NAN_GETTER(featuresGetter);
	static NAN_GETTER(fieldsGetter);
	static NAN_GETTER(nameGetter);
	static NAN_GETTER(fidColumnGetter);
	static NAN_GETTER(geomColumnGetter);
	static NAN_GETTER(geomTypeGetter);

	static ObjectCache<OGRLayer, Layer> cache;

	Layer();
	Layer(OGRLayer *ds);
	inline OGRLayer *get() {
		return this_;
	}
	#if GDAL_VERSION_MAJOR >= 2
	inline GDALDataset *getParent() {
		return parent_ds;
	}
	#else 
	inline OGRDataSource *getParent() {
		return parent_ds;
	}
	#endif
	void dispose();

private:
	~Layer();
	OGRLayer *this_;
	#if GDAL_VERSION_MAJOR >= 2
	GDALDataset *parent_ds;
	#else
	OGRDataSource *parent_ds;
	#endif 
	bool is_result_set;
};

}
#endif
