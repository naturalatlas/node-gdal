#ifndef __NODE_OGR_LAYER_H__
#define __NODE_OGR_LAYER_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>

#include "obj_cache.hpp"
#include "gdal_dataset.hpp"

using namespace v8;
using namespace node;

namespace node_gdal {

class Layer: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	#if GDAL_VERSION_MAJOR >= 2
	static Handle<Value> New(OGRLayer *raw, GDALDataset *raw_parent);
	static Handle<Value> New(OGRLayer *raw, GDALDataset *raw_parent, bool result_set);
	#else
	static Handle<Value> New(OGRLayer *raw, OGRDataSource *raw_parent);
	static Handle<Value> New(OGRLayer *raw, OGRDataSource *raw_parent, bool result_set);
	#endif
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> getExtent(const Arguments &args);
	static Handle<Value> setAttributeFilter(const Arguments &args);
	static Handle<Value> setSpatialFilter(const Arguments &args);
	static Handle<Value> getSpatialFilter(const Arguments &args);
	static Handle<Value> testCapability(const Arguments &args);
	static Handle<Value> syncToDisk(const Arguments &args);

	static void dsSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static Handle<Value> dsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> srsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> featuresGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> fieldsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> nameGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> fidColumnGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> geomColumnGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> geomTypeGetter(Local<String> property, const AccessorInfo &info);

	static ObjectCache<OGRLayer*> cache;

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
