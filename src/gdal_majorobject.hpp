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

class MajorObject: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	static Handle<Value> New(GDALMajorObject *obj);
	static NAN_METHOD(getMetadata);
	static Handle<Object> getMetadata(GDALMajorObject *obj, const char *domain);
	
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
