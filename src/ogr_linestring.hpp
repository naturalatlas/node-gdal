#ifndef __NODE_OGR_LINESTRING_H__
#define __NODE_OGR_LINESTRING_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>
#include "ogr_geometry.hpp"

using namespace v8;
using namespace node;

namespace node_ogr {

class LineString: public node::ObjectWrap {

  public:
    static Persistent<FunctionTemplate> constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRLineString *geom);
    static Handle<Value> New(OGRLineString *geom, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> addPoint(const Arguments &args);
    static Handle<Value> getPoint(const Arguments &args);
    static Handle<Value> getLength(const Arguments &args);
    static Handle<Value> getNumPoints(const Arguments &args);
    static Handle<Value> value(const Arguments &args);

    LineString();
    LineString(OGRLineString *geom);
    inline OGRLineString *get() { return this_; }

  private:
    ~LineString();
    OGRLineString *this_;
    bool owned_;
    int size_;
};

}

#endif
