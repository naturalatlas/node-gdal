#ifndef __NODE_GDAL_BAND_PIXELS_H__
#define __NODE_GDAL_BAND_PIXELS_H__

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

class RasterBandPixels: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(Handle<Value> band_obj);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> get(const Arguments &args);
	static Handle<Value> set(const Arguments &args);
	static Handle<Value> read(const Arguments &args);
	static Handle<Value> write(const Arguments &args);
	static Handle<Value> readBlock(const Arguments &args);
	static Handle<Value> writeBlock(const Arguments &args);
	
	RasterBandPixels();
private:
	~RasterBandPixels();
};

}
#endif
