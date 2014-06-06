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
    static Persistent<FunctionTemplate> point_constructor;
    static Persistent<FunctionTemplate> polygon_constructor;
    static Persistent<FunctionTemplate> linearring_constructor;
    static Persistent<FunctionTemplate> linestring_constructor;
    static Persistent<FunctionTemplate> geometrycollection_constructor;
    static Persistent<FunctionTemplate> multipoint_constructor;
    static Persistent<FunctionTemplate> multilinestring_constructor;
    static Persistent<FunctionTemplate> multipolygon_constructor;

    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> NewPoint(const Arguments &args);
    static Handle<Value> NewPolygon(const Arguments &args);
    static Handle<Value> NewLinearRing(const Arguments &args);
    static Handle<Value> NewLineString(const Arguments &args);
    static Handle<Value> NewGeometryCollection(const Arguments &args);
    static Handle<Value> NewMultiPoint(const Arguments &args);
    static Handle<Value> NewMultiLineString(const Arguments &args);
    static Handle<Value> NewMultiPolygon(const Arguments &args);
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
    static Handle<Value> unionCascaded(const Arguments &args);
    static Handle<Value> difference(const Arguments &args);
    static Handle<Value> symDifference(const Arguments &args);
    static Handle<Value> centroid(const Arguments &args);
    static Handle<Value> simplify(const Arguments &args);
    static Handle<Value> simplifyPreserveTopology(const Arguments &args);
    static Handle<Value> polygonize(const Arguments &args);
    static Handle<Value> swapXY(const Arguments &args);
    static Handle<Value> getArea(const Arguments &args);
    static Handle<Value> addPoint(const Arguments &args);
    static Handle<Value> addGeometry(const Arguments &args);
    static Handle<Value> getGeometry(const Arguments &args);
    static Handle<Value> getNumGeometries(const Arguments &args);
    static Handle<Value> getEnvelope(const Arguments &args);
    static Handle<Value> getEnvelope3D(const Arguments &args);
    static Handle<Value> getLength(const Arguments &args);
    static Handle<Value> value(const Arguments &args);
    static Handle<Value> getPoint(const Arguments &args);
    static Handle<Value> getX(const Arguments &args);
    static Handle<Value> getY(const Arguments &args);
    static Handle<Value> getZ(const Arguments &args);
    static Handle<Value> setX(const Arguments &args);
    static Handle<Value> setY(const Arguments &args);
    static Handle<Value> setZ(const Arguments &args);
    static Handle<Value> transform(const Arguments &args);
    static Handle<Value> transformTo(const Arguments &args);
    static Handle<Value> getSpatialReference(const Arguments &args);
    static Handle<Value> assignSpatialReference(const Arguments &args);

    static OGRwkbGeometryType getGeometryType_fixed(Geometry* geom);
    static void updateAmountOfExternalAllocatedMemory(Geometry* geom);

    Geometry();
    Geometry(OGRGeometry *geom);
    inline OGRGeometry *get() { return this_; }

  private:
    ~Geometry();
    OGRGeometry *this_;
    bool owned_;
    int size_;
};

}

#endif
