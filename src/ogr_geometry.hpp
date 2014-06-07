#ifndef __NODE_OGR_GEOMETRY_H__
#define __NODE_OGR_GEOMETRY_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

namespace node_ogr {

class Geometry: public node::ObjectWrap {
  friend class Feature;

  public:
    static Persistent<FunctionTemplate> constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRGeometry *geom);
    static Handle<Value> New(OGRGeometry *geom, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> getDimension(const Arguments &args);
    static Handle<Value> getCoordinateDimension(const Arguments &args);
    static Handle<Value> isEmpty(const Arguments &args);
    static Handle<Value> isValid(const Arguments &args);
    static Handle<Value> isSimple(const Arguments &args);
    static Handle<Value> isRing(const Arguments &args);
    static Handle<Value> clone(const Arguments &args);
    static Handle<Value> empty(const Arguments &args);
    static Handle<Value> wkbSize(const Arguments &args);
    static Handle<Value> getGeometryType(const Arguments &args);
    static Handle<Value> getGeometryName(const Arguments &args);
    static Handle<Value> exportToKML(const Arguments &args);
    static Handle<Value> exportToGML(const Arguments &args);
    static Handle<Value> exportToJSON(const Arguments &args);
    static Handle<Value> exportToWKT(const Arguments &args);
    static Handle<Value> closeRings(const Arguments &args);
    static Handle<Value> segmentize(const Arguments &args);
    static Handle<Value> intersects(const Arguments &args);
    static Handle<Value> equals(const Arguments &args);
    static Handle<Value> disjoint(const Arguments &args);
    static Handle<Value> touches(const Arguments &args);
    static Handle<Value> crosses(const Arguments &args);
    static Handle<Value> within(const Arguments &args);
    static Handle<Value> contains(const Arguments &args);
    static Handle<Value> overlaps(const Arguments &args);
    static Handle<Value> boundary(const Arguments &args);
    static Handle<Value> distance(const Arguments &args);
    static Handle<Value> convexHull(const Arguments &args);
    static Handle<Value> buffer(const Arguments &args);
    static Handle<Value> intersection(const Arguments &args);
    static Handle<Value> unionGeometry(const Arguments &args);
    static Handle<Value> difference(const Arguments &args);
    static Handle<Value> symDifference(const Arguments &args);
    static Handle<Value> centroid(const Arguments &args);
    static Handle<Value> simplify(const Arguments &args);
    static Handle<Value> simplifyPreserveTopology(const Arguments &args);
    static Handle<Value> polygonize(const Arguments &args);
    static Handle<Value> swapXY(const Arguments &args);
    static Handle<Value> getNumGeometries(const Arguments &args);
    static Handle<Value> getEnvelope(const Arguments &args);
    static Handle<Value> getEnvelope3D(const Arguments &args);
    static Handle<Value> transform(const Arguments &args);
    static Handle<Value> transformTo(const Arguments &args);
    static Handle<Value> getSpatialReference(const Arguments &args);
    static Handle<Value> assignSpatialReference(const Arguments &args);

    //static constructor methods
    static Handle<Value> create(const Arguments &args);
    static Handle<Value> createFromWkt(const Arguments &args);

    static OGRwkbGeometryType getGeometryType_fixed(OGRGeometry* geom);

    Geometry();
    Geometry(OGRGeometry *geom);
    inline OGRGeometry *get() { return this_; }

  protected:
    ~Geometry();
    OGRGeometry *this_;
    bool owned_;
    int size_;
};

}

#define UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(geom) {                                        \
    int new_size = geom->this_->WkbSize();                                              \
    if(geom->owned_) V8::AdjustAmountOfExternalAllocatedMemory(new_size - geom->size_); \
    geom->size_ = new_size;                                                             \
}

#include "ogr_point.hpp"
#include "ogr_linestring.hpp"
#include "ogr_linearring.hpp"
#include "ogr_polygon.hpp"
#include "ogr_geometrycollection.hpp"
#include "ogr_multipoint.hpp"
#include "ogr_multilinestring.hpp"
#include "ogr_multipolygon.hpp"

#endif
