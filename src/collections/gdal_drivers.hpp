#ifndef __NODE_GDAL_DRIVERS_H__
#define __NODE_GDAL_DRIVERS_H__

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

class GDALDrivers: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New();
	static NAN_METHOD(toString);

	static NAN_METHOD(get);
	static NAN_METHOD(getNames);
	static NAN_METHOD(count);

	GDALDrivers();
private:
	~GDALDrivers();
};

}
#endif
