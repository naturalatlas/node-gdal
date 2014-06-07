
#include "ogr_common.hpp"

#include "ogr_spatial_reference.hpp"
#include "ogr_coordinate_transformation.hpp"
#include "ogr_geometry.hpp"

#include <sstream>

using namespace node_ogr;

Persistent<FunctionTemplate> Geometry::constructor;

void Geometry::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Geometry::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("Geometry"));

  NODE_SET_METHOD(constructor, "create", Geometry::create);
  NODE_SET_METHOD(constructor, "createFromWkt", Geometry::createFromWkt);

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getDimension", getDimension);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getCoordinateDimension", getCoordinateDimension);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isEmpty", isEmpty);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isValid", isValid);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isSimple", isSimple);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isRing", isRing);
  NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
  NODE_SET_PROTOTYPE_METHOD(constructor, "empty", empty);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometryType", getGeometryType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometryName", getGeometryName);
  NODE_SET_PROTOTYPE_METHOD(constructor, "exportToKML", exportToKML);
  NODE_SET_PROTOTYPE_METHOD(constructor, "exportToGML", exportToGML);
  NODE_SET_PROTOTYPE_METHOD(constructor, "exportToJSON", exportToJSON);
  NODE_SET_PROTOTYPE_METHOD(constructor, "exportToWKT", exportToWKT);
  NODE_SET_PROTOTYPE_METHOD(constructor, "closeRings", closeRings);
  NODE_SET_PROTOTYPE_METHOD(constructor, "intersects", intersects);
  NODE_SET_PROTOTYPE_METHOD(constructor, "equals", equals);
  NODE_SET_PROTOTYPE_METHOD(constructor, "disjoint", disjoint);
  NODE_SET_PROTOTYPE_METHOD(constructor, "touches", touches);
  NODE_SET_PROTOTYPE_METHOD(constructor, "crosses", crosses);
  NODE_SET_PROTOTYPE_METHOD(constructor, "within", within);
  NODE_SET_PROTOTYPE_METHOD(constructor, "contains", contains);
  NODE_SET_PROTOTYPE_METHOD(constructor, "overlaps", overlaps);
  NODE_SET_PROTOTYPE_METHOD(constructor, "boundary", boundary);
  NODE_SET_PROTOTYPE_METHOD(constructor, "distance", distance);
  NODE_SET_PROTOTYPE_METHOD(constructor, "convexHull", convexHull);
  NODE_SET_PROTOTYPE_METHOD(constructor, "buffer", buffer);
  NODE_SET_PROTOTYPE_METHOD(constructor, "intersection", intersection);
  NODE_SET_PROTOTYPE_METHOD(constructor, "union", unionGeometry);
  NODE_SET_PROTOTYPE_METHOD(constructor, "difference", difference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "symDifference", symDifference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "centroid", centroid);
  NODE_SET_PROTOTYPE_METHOD(constructor, "simplify", simplify);
  NODE_SET_PROTOTYPE_METHOD(constructor, "simplifyPreserveTopology", simplifyPreserveTopology);
  NODE_SET_PROTOTYPE_METHOD(constructor, "segmentize", segmentize);
  NODE_SET_PROTOTYPE_METHOD(constructor, "swapXY", swapXY);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getEnvelope", getEnvelope);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getEnvelope3D", getEnvelope3D);
  NODE_SET_PROTOTYPE_METHOD(constructor, "transform", transform);
  NODE_SET_PROTOTYPE_METHOD(constructor, "transformTo", transformTo);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getSpatialReference", getSpatialReference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "assignSpatialReference", assignSpatialReference);

  //change to getter
  NODE_SET_PROTOTYPE_METHOD(constructor, "wkbSize", wkbSize);

  target->Set(String::NewSymbol("Geometry"), constructor->GetFunction());
}

Geometry::Geometry(OGRGeometry *geom)
: ObjectWrap(),
  this_(geom),
  owned_(true),
  size_(0)
{}

Geometry::Geometry()
: ObjectWrap(),
  this_(NULL),
  owned_(true),
  size_(0)
{
}

Geometry::~Geometry()
{
  if (owned_ && this_) {
    OGRGeometryFactory::destroyGeometry(this_);
    V8::AdjustAmountOfExternalAllocatedMemory(-size_);
  }
  this_ = NULL;
}

Handle<Value> Geometry::New(const Arguments& args)
{
  HandleScope scope;
  Geometry *f;

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void* ptr = ext->Value();
    f = static_cast<Geometry *>(ptr);

  }else{
    return NODE_THROW("Geometry doesnt have a constructor, use Geometry.createFromWkt(), Geometry.create() or type-specific constructor. ie. new ogr.Point()");
    OGRwkbGeometryType geometry_type;
    NODE_ARG_ENUM(0, "geometry type", OGRwkbGeometryType, geometry_type);
    OGRGeometry *geom = OGRGeometryFactory::createGeometry(geometry_type);
    f = new Geometry(geom);
  }

  f->Wrap(args.This());
  return args.This();
}

Handle<Value> Geometry::New(OGRGeometry *geom) {
  HandleScope scope;
  return scope.Close(Geometry::New(geom, true));
}

Handle<Value> Geometry::New(OGRGeometry *geom, bool owned) {
  HandleScope scope;

  if(!geom) return Null();

  switch(getGeometryType_fixed(geom)){
    case wkbPoint              : 
      return scope.Close(Point::New(static_cast<OGRPoint*>(geom), owned));
    case wkbLineString         : 
      return scope.Close(LineString::New(static_cast<OGRLineString*>(geom), owned));
    case wkbLinearRing         : 
      return scope.Close(LinearRing::New(static_cast<OGRLinearRing*>(geom), owned));
    case wkbPolygon            : 
      return scope.Close(Polygon::New(static_cast<OGRPolygon*>(geom), owned));
    case wkbGeometryCollection :
      return scope.Close(GeometryCollection::New(static_cast<OGRGeometryCollection*>(geom), owned));
    case wkbMultiPoint         : 
      return scope.Close(MultiPoint::New(static_cast<OGRMultiPoint*>(geom), owned));
    case wkbMultiLineString    : 
      return scope.Close(LineString::New(static_cast<OGRLineString*>(geom), owned));
    case wkbMultiPolygon       : 
      return scope.Close(MultiPolygon::New(static_cast<OGRMultiPolygon*>(geom), owned));
    default                    : 
      return NODE_THROW("Tried to create unsupported geometry type");
  }
}

OGRwkbGeometryType Geometry::getGeometryType_fixed(OGRGeometry* geom)
{
  //For some reason OGRLinearRing::getGeometryType uses OGRLineString's method...
  //meaning OGRLinearRing::getGeometryType returns wkbLineString

  //http://trac.osgeo.org/gdal/ticket/1755

  OGRwkbGeometryType type =  wkbFlatten(geom->getGeometryType());

  if(type == wkbLineString){
    if(std::string(geom->getGeometryName()) == "LINEARRING"){
      return wkbLinearRing;
    }  
  }

  return type;
}

Handle<Value> Geometry::toString(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  std::ostringstream ss;
  ss << "Geometry (" << geom->this_->getGeometryName() << ")";
  return scope.Close(String::New(ss.str().c_str()));
}


NODE_WRAPPED_METHOD(Geometry, closeRings, closeRings);
NODE_WRAPPED_METHOD(Geometry, empty, empty);
NODE_WRAPPED_METHOD(Geometry, swapXY, swapXY);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, getDimension, Integer, getDimension);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, getCoordinateDimension, Integer, getCoordinateDimension);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, isEmpty, Boolean, IsEmpty);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, isValid, Boolean, IsValid);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, isSimple, Boolean, IsSimple);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, isRing, Boolean, IsRing);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, clone, Geometry, clone);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, wkbSize, Integer, WkbSize);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, getGeometryType, Integer, getGeometryType);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, getGeometryName, SafeString, getGeometryName);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, exportToKML, SafeString, exportToKML);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, exportToGML, SafeString, exportToGML);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, exportToJSON, SafeString, exportToJson);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, convexHull, Geometry, ConvexHull);
NODE_WRAPPED_METHOD_WITH_RESULT(Geometry, boundary, Geometry, Boundary);
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, intersects, Boolean, Intersects, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, equals, Boolean, Equals, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, disjoint, Boolean, Disjoint, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, touches, Boolean, Touches, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, crosses, Boolean, Crosses, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, within, Boolean, Within, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, contains, Boolean, Contains, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, overlaps, Boolean, Overlaps, Geometry, "geometry to compare");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, distance, Number, Distance, Geometry, "geometry to use for distance calculation");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, intersection, Geometry, Intersection, Geometry, "geometry to use for intersection");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, unionGeometry, Geometry, Union, Geometry, "geometry to use for union");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, difference, Geometry, Difference, Geometry, "geometry to use for difference");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Geometry, symDifference, Geometry, SymDifference, Geometry, "geometry to use for sym difference");
NODE_WRAPPED_METHOD_WITH_RESULT_1_DOUBLE_PARAM(Geometry, simplify, Geometry, Simplify, "tolerance");
NODE_WRAPPED_METHOD_WITH_RESULT_1_DOUBLE_PARAM(Geometry, simplifyPreserveTopology, Geometry, SimplifyPreserveTopology, "tolerance");
NODE_WRAPPED_METHOD_WITH_1_DOUBLE_PARAM(Geometry, segmentize, segmentize, "segment length");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Geometry, transform, transform, CoordinateTransformation, "transform");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Geometry, transformTo, transformTo, SpatialReference, "spatial reference");
NODE_WRAPPED_METHOD_WITH_1_WRAPPED_PARAM(Geometry, assignSpatialReference, assignSpatialReference, SpatialReference, "spatial reference");

//manually wrap this method because we don't have macros for multiple params
Handle<Value> Geometry::buffer(const Arguments& args)
{
  HandleScope scope;

  double distance;
  int number_of_segments = 30;

  NODE_ARG_DOUBLE(0, "distance", distance);
  NODE_ARG_INT_OPT(1, "number of segments", number_of_segments);

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  return scope.Close(Geometry::New(geom->this_->Buffer(distance, number_of_segments)));
}


Handle<Value> Geometry::exportToWKT(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  char *text = NULL;
  geom->this_->exportToWkt(&text);

  if (text) {
    return scope.Close(SafeString::New(text));
  }

  return Undefined();
}

// The Centroid method wants the caller to create the point to fill in. Instead
// of requiring the caller to create the point geometry to fill in, we new up an
// OGRPoint and put the result into it and return that.
Handle<Value> Geometry::centroid(const Arguments& args)
{
  HandleScope scope;
  OGRPoint *point = new OGRPoint();

  ObjectWrap::Unwrap<Geometry>(args.This())->this_->Centroid(point);

  return scope.Close(Point::New(point));
}

Handle<Value> Geometry::getEnvelope(const Arguments& args)
{
  //returns object containing boundaries until complete OGREnvelope binding is built

  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  OGREnvelope *envelope = new OGREnvelope();
  geom->this_->getEnvelope(envelope);

  Local<Object> obj = Object::New();
  obj->Set(String::NewSymbol("minX"), Number::New(envelope->MinX));
  obj->Set(String::NewSymbol("maxX"), Number::New(envelope->MaxX));
  obj->Set(String::NewSymbol("minY"), Number::New(envelope->MinY));
  obj->Set(String::NewSymbol("maxY"), Number::New(envelope->MaxY));

  delete envelope;

  return scope.Close(obj);
}

Handle<Value> Geometry::getEnvelope3D(const Arguments& args)
{
  //returns object containing boundaries until complete OGREnvelope binding is built

  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  OGREnvelope3D *envelope = new OGREnvelope3D();
  geom->this_->getEnvelope(envelope);

  Local<Object> obj = Object::New();
  obj->Set(String::NewSymbol("minX"), Number::New(envelope->MinX));
  obj->Set(String::NewSymbol("maxX"), Number::New(envelope->MaxX));
  obj->Set(String::NewSymbol("minY"), Number::New(envelope->MinY));
  obj->Set(String::NewSymbol("maxY"), Number::New(envelope->MaxY));
  obj->Set(String::NewSymbol("minZ"), Number::New(envelope->MinZ));
  obj->Set(String::NewSymbol("maxZ"), Number::New(envelope->MaxZ));

  delete envelope;

  return scope.Close(obj);
}

Handle<Value> Geometry::getSpatialReference(const Arguments& args)
{
  HandleScope scope;
  
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  return scope.Close(SpatialReference::New(geom->this_->getSpatialReference(), false));
}

// --- JS static methods (OGRGeometryFactory) ---

Handle<Value> Geometry::createFromWkt(const Arguments &args) {
  HandleScope scope;

  std::string wkt_string; 
  SpatialReference *srs = NULL;

  NODE_ARG_STR(0, "wkt", wkt_string);
  NODE_ARG_WRAPPED_OPT(1, "srs", SpatialReference, srs);

  char *wkt = (char*) wkt_string.c_str(); 
  OGRGeometry *geom = NULL;
  OGRSpatialReference *ogr_srs = NULL;
  if(srs) 
    ogr_srs = srs->get();

  if(OGRGeometryFactory::createFromWkt(&wkt, ogr_srs, &geom)){
    return NODE_THROW("Error creating geometry");
  }

  return scope.Close(Geometry::New(geom, true));
}

Handle<Value> Geometry::create(const Arguments &args) {
  HandleScope scope;

  OGRwkbGeometryType type = wkbUnknown;
  NODE_ARG_ENUM(0, "type", OGRwkbGeometryType, type);

  return scope.Close(Geometry::New(OGRGeometryFactory::createGeometry(type), true));
}