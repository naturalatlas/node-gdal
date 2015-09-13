#ifndef __NODE_GDAL_FIELD_DEFN_COLLECTION_H__
#define __NODE_GDAL_FIELD_DEFN_COLLECTION_H__

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

// FeatureDefn.fields : FeatureDefnFields

namespace node_gdal {

class FeatureDefnFields: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(Local<Value> layer_obj);
	static NAN_METHOD(toString);

	static NAN_METHOD(get);
	static NAN_METHOD(getNames);
	static NAN_METHOD(count);
	static NAN_METHOD(add);
	static NAN_METHOD(remove);
	static NAN_METHOD(indexOf);
	static NAN_METHOD(reorder);

	// - implement in the future -
	//static NAN_METHOD(alter);

	static NAN_GETTER(featureDefnGetter);

	FeatureDefnFields();
private:
	~FeatureDefnFields();
};

}
#endif
