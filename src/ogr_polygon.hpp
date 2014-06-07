#ifndef __NODE_OGR_POLY_H__
#define __NODE_OGR_POLY_H__

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

class Polygon: public node::ObjectWrap {

  public:
    static Persistent<FunctionTemplate> constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRPolygon *geom);
    static Handle<Value> New(OGRPolygon *geom, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> getArea(const Arguments &args);
    static Handle<Value> addRing(const Arguments &args);
    static Handle<Value> getExteriorRing(const Arguments &args);
    static Handle<Value> getInteriorRing(const Arguments &args);
    static Handle<Value> getNumInteriorRings(const Arguments &args);

    Polygon();
    Polygon(OGRPolygon *geom);
    inline OGRPolygon *get() { return this_; }

  private:
    ~Polygon();
    OGRPolygon *this_;
    bool owned_;
    int size_;
};

}

#endif
