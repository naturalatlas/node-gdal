#ifndef __NODE_GDAL_DRIVER_H__
#define __NODE_GDAL_DRIVER_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

class Driver: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALDriver *driver);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> create(const Arguments &args);
	static Handle<Value> createCopy(const Arguments &args);
	static Handle<Value> deleteDataset(const Arguments &args);
	static Handle<Value> quietDelete(const Arguments &args);
	static Handle<Value> rename(const Arguments &args);
	static Handle<Value> copyFiles(const Arguments &args);
	static Handle<Value> getMetadata(const Arguments &args);

	static Handle<Value> shortNameGetter(Local<String> property, const AccessorInfo& info);
	static Handle<Value> longNameGetter(Local<String> property, const AccessorInfo& info);

	Driver();
	Driver(GDALDriver *ds);
	inline GDALDriver *get() {
		return this_;
	}

private:
	~Driver();
	GDALDriver *this_;
};

#endif
