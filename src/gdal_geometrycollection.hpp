#ifndef __NODE_OGR_GEOMETRYCOLLECTION_H__
#define __NODE_OGR_GEOMETRYCOLLECTION_H__

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

class GeometryCollection: public Nan::ObjectWrap {

public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRGeometryCollection *geom);
	static Local<Value> New(OGRGeometryCollection *geom, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(getArea);
	static NAN_METHOD(getLength);

	static NAN_GETTER(childrenGetter);

	GeometryCollection();
	GeometryCollection(OGRGeometryCollection *geom);
	inline OGRGeometryCollection *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

private:
	~GeometryCollection();
	OGRGeometryCollection *this_;
	bool owned_;
	int size_;
};

}
#endif
