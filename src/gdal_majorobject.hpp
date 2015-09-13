#ifndef __NODE_GDAL_MAJOROBJECT_H__
#define __NODE_GDAL_MAJOROBJECT_H__

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

class MajorObject: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(GDALMajorObject *obj);
	static NAN_METHOD(getMetadata);
	static Local<Object> getMetadata(GDALMajorObject *obj, const char *domain);
	
	static NAN_GETTER(descriptionGetter);

	MajorObject();
	MajorObject(GDALMajorObject *obj);
	inline GDALMajorObject *get() {
		return this_;
	}

private:
	~MajorObject();
	GDALMajorObject *this_;
};

}
#endif
