#ifndef __NODE_OGR_MULTILINESTRING_H__
#define __NODE_OGR_MULTILINESTRING_H__

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

class MultiLineString: public node::ObjectWrap {

  public:
    static Persistent<FunctionTemplate> constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRMultiLineString *geom);
    static Handle<Value> New(OGRMultiLineString *geom, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> polygonize(const Arguments &args);
    static Handle<Value> getGeometry(const Arguments &args);


    MultiLineString();
    MultiLineString(OGRMultiLineString *geom);
    inline OGRMultiLineString *get() { return this_; }

  private:
    ~MultiLineString();
    OGRMultiLineString *this_;
    bool owned_;
    int size_;
};

}

#endif
