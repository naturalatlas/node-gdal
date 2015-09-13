#ifndef __NODE_GDAL_BAND_PIXELS_H__
#define __NODE_GDAL_BAND_PIXELS_H__

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

class RasterBandPixels: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(Local<Value> band_obj);
	static NAN_METHOD(toString);

	static NAN_METHOD(get);
	static NAN_METHOD(set);
	static NAN_METHOD(read);
	static NAN_METHOD(write);
	static NAN_METHOD(readBlock);
	static NAN_METHOD(writeBlock);
	
	RasterBandPixels();
private:
	~RasterBandPixels();
};

}
#endif
