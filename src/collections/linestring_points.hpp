#ifndef __NODE_GDAL_LINESTRING_POINTS_H__
#define __NODE_GDAL_LINESTRING_POINTS_H__

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

// LineString.children

namespace node_gdal {

class LineStringPoints: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(Local<Value> geom);
	static NAN_METHOD(toString);

	static NAN_METHOD(add);
	static NAN_METHOD(get);
	static NAN_METHOD(set);
	static NAN_METHOD(count);
	static NAN_METHOD(reverse);
	static NAN_METHOD(resize);

	LineStringPoints();
private:
	~LineStringPoints();
};

}
#endif
