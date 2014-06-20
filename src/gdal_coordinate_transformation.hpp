#ifndef __NODE_OGR_COORDINATETRANSFORMATION_H__
#define __NODE_OGR_COORDINATETRANSFORMATION_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class CoordinateTransformation: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(OGRCoordinateTransformation *transform);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> transformPoint(const Arguments &args);

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
