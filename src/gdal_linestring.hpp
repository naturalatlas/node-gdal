#ifndef __NODE_OGR_LINESTRING_H__
#define __NODE_OGR_LINESTRING_H__

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

class LineString: public Nan::ObjectWrap {

public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRLineString *geom);
	static Local<Value> New(OGRLineString *geom, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(getLength);
	static NAN_METHOD(value);
	static NAN_METHOD(addSubLineString);

	static NAN_GETTER(pointsGetter);

	LineString();
	LineString(OGRLineString *geom);
	inline OGRLineString *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

private:
	~LineString();
	OGRLineString *this_;
	bool owned_;
	int size_;
};

}
#endif
