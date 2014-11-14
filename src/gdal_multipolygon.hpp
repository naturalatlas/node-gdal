#ifndef __NODE_OGR_MULTIPOLYGON_H__
#define __NODE_OGR_MULTIPOLYGON_H__

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

class MultiPolygon: public node::ObjectWrap {

public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	static Handle<Value> New(OGRMultiPolygon *geom);
	static Handle<Value> New(OGRMultiPolygon *geom, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(unionCascaded);
	static NAN_METHOD(getArea);

	MultiPolygon();
	MultiPolygon(OGRMultiPolygon *geom);
	inline OGRMultiPolygon *get() {
		return this_;
	}

private:
	~MultiPolygon();
	OGRMultiPolygon *this_;
	bool owned_;
	int size_;
};

}
#endif
