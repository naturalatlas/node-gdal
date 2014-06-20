#ifndef __NODE_GDAL_DRIVERS_H__
#define __NODE_GDAL_DRIVERS_H__

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

class GDALDrivers: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New();
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> get(const Arguments &args);
	static Handle<Value> getNames(const Arguments &args);
	static Handle<Value> count(const Arguments &args);

	GDALDrivers();
private:
	~GDALDrivers();
};

}
#endif
