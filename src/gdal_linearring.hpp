#ifndef __NODE_OGR_LINEARRING_H__
#define __NODE_OGR_LINEARRING_H__

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

class LinearRing: public Nan::ObjectWrap {

public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRLinearRing *geom);
	static Local<Value> New(OGRLinearRing *geom, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(getArea);

	LinearRing();
	LinearRing(OGRLinearRing *geom);
	inline OGRLinearRing *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

private:
	~LinearRing();
	OGRLinearRing *this_;
	bool owned_;
	int size_;
};

}
#endif
