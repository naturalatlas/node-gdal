#ifndef __NODE_GDAL_LAYER_COLLECTION_H__
#define __NODE_GDAL_LAYER_COLLECTION_H__

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

class DatasetLayers: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(Local<Value> ds_obj);
	static NAN_METHOD(toString);

	static NAN_METHOD(get);
	static NAN_METHOD(count);
	static NAN_METHOD(create);
	static NAN_METHOD(copy);
	static NAN_METHOD(remove);

	static NAN_GETTER(dsGetter);

	DatasetLayers();
private:
	~DatasetLayers();
};

}
#endif
