#ifndef __NODE_OGR_GEOMETRYCOLLECTION_H__
#define __NODE_OGR_GEOMETRYCOLLECTION_H__

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

class GeometryCollection: public node::ObjectWrap {

  public:
    static Persistent<FunctionTemplate> constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRGeometryCollection *geom);
    static Handle<Value> New(OGRGeometryCollection *geom, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> getArea(const Arguments &args);
    static Handle<Value> getLength(const Arguments &args);
    static Handle<Value> addGeometry(const Arguments &args);
    static Handle<Value> getGeometry(const Arguments &args);
    static Handle<Value> getNumGeometries(const Arguments &args);

    GeometryCollection();
    GeometryCollection(OGRGeometryCollection *geom);
    inline OGRGeometryCollection *get() { return this_; }

  private:
    ~GeometryCollection();
    OGRGeometryCollection *this_;
    bool owned_;
    int size_;
};

}

#endif
