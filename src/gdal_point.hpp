#ifndef __NODE_OGR_POINT_H__
#define __NODE_OGR_POINT_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>
#include "gdal_geometry.hpp"

using namespace v8;
using namespace node;

class Point: public node::ObjectWrap {

public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(OGRPoint *geom);
	static Handle<Value> New(OGRPoint *geom, bool owned);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> xGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> yGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> zGetter(Local<String> property, const AccessorInfo &info);
	static void xSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void ySetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void zSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);

	Point();
	Point(OGRPoint *geom);
	inline OGRPoint *get() {
		return this_;
	}

private:
	~Point();
	OGRPoint *this_;
	bool owned_;
	int size_;
};

#endif
