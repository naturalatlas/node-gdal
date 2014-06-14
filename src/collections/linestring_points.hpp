#ifndef __NODE_GDAL_LINESTRING_POINTS_H__
#define __NODE_GDAL_LINESTRING_POINTS_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

// LineString.children

class LineStringPoints: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(Handle<Value> geom);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> add(const Arguments &args);
	static Handle<Value> get(const Arguments &args);
	static Handle<Value> set(const Arguments &args);
	static Handle<Value> count(const Arguments &args);
	static Handle<Value> reverse(const Arguments &args);
	static Handle<Value> resize(const Arguments &args);

	LineStringPoints();
private:
	~LineStringPoints();
};

#endif
