#ifndef __NODE_OGR_COORDINATETRANSFORMATION_H__
#define __NODE_OGR_COORDINATETRANSFORMATION_H__

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

class CoordinateTransformation: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static NAN_METHOD(New);
	static Handle<Value> New(OGRCoordinateTransformation *transform);
	static NAN_METHOD(toString);
	static NAN_METHOD(transformPoint);

	CoordinateTransformation();
	CoordinateTransformation(OGRCoordinateTransformation *srs);
	inline OGRCoordinateTransformation *get() {
		return this_;
	}

private:
	~CoordinateTransformation();
	OGRCoordinateTransformation *this_;
};

}
#endif
