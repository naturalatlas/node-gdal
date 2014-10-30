#ifndef __NODE_GDAL_FIELD_COLLECTION_H__
#define __NODE_GDAL_FIELD_COLLECTION_H__

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

using namespace v8;
using namespace node;

namespace node_gdal {

class FeatureFields: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	static Handle<Value> New(Handle<Value> layer_obj);
	static NAN_METHOD(toString);
	static NAN_METHOD(toArray);
	static NAN_METHOD(toObject);

	static NAN_METHOD(get);
	static NAN_METHOD(getNames);
	static NAN_METHOD(set);
	static NAN_METHOD(reset);
	static NAN_METHOD(count);
	static NAN_METHOD(indexOf);

	static Handle<Value> get(OGRFeature *f, int field_index);
	static Handle<Value> getFieldAsIntegerList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsDoubleList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsStringList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsBinary(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsDateTime(OGRFeature* feature, int field_index);

	static NAN_GETTER(featureGetter);

	FeatureFields();
private:
	~FeatureFields();
};

}
#endif
