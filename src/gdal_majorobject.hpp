#ifndef __NODE_GDAL_MAJOROBJECT_H__
#define __NODE_GDAL_MAJOROBJECT_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class MajorObject: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALMajorObject *obj);
	static Handle<Value> getMetadata(const Arguments &args);
	static Handle<Value> getMetadata(GDALMajorObject *obj, const char *domain);
	
	static Handle<Value> descriptionGetter(Local<String> property, const AccessorInfo &info);

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
