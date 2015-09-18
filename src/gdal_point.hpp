#ifndef __NODE_OGR_POINT_H__
#define __NODE_OGR_POINT_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class Point: public Nan::ObjectWrap {

public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRPoint *geom);
	static Local<Value> New(OGRPoint *geom, bool owned);
	static NAN_METHOD(toString);

	static NAN_GETTER(xGetter);
	static NAN_GETTER(yGetter);
	static NAN_GETTER(zGetter);
	static NAN_SETTER(xSetter);
	static NAN_SETTER(ySetter);
	static NAN_SETTER(zSetter);

	Point();
	Point(OGRPoint *geom);
	inline OGRPoint *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}
	
private:
	~Point();
	OGRPoint *this_;
	bool owned_;
	int size_;
};

}
#endif
