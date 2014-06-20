#ifndef __NODE_GDAL_POLYGON_RINGS_H__
#define __NODE_GDAL_POLYGON_RINGS_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

// Polygon.rings

namespace node_gdal {

class PolygonRings: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(Handle<Value> geom);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> get(const Arguments &args);
	static Handle<Value> count(const Arguments &args);
	static Handle<Value> add(const Arguments &args);
	static Handle<Value> remove(const Arguments &args);

	PolygonRings();
private:
	~PolygonRings();
};

}
#endif
