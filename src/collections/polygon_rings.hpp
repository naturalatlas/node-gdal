#ifndef __NODE_GDAL_POLYGON_RINGS_H__
#define __NODE_GDAL_POLYGON_RINGS_H__

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

// Polygon.rings

namespace node_gdal {

class PolygonRings: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(Local<Value> geom);
	static NAN_METHOD(toString);

	static NAN_METHOD(get);
	static NAN_METHOD(count);
	static NAN_METHOD(add);
	static NAN_METHOD(remove);

	PolygonRings();
private:
	~PolygonRings();
};

}
#endif
