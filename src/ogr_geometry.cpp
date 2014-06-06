
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"
#include "ogr_spatial_reference.hpp"
#include "ogr_coordinate_transformation.hpp"
#include <sstream>

using namespace node_ogr;

Persistent<FunctionTemplate> Geometry::constructor;

void Geometry::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Geometry::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("Geometry"));

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getDimension", getDimension);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getCoordinateDimension", getCoordinateDimension);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isEmpty", isEmpty);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isValid", isValid);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isSimple", isSimple);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isRing", isRing);
  NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
  NODE_SET_PROTOTYPE_METHOD(constructor, "empty", empty);
  NODE_SET_PROTOTYPE_METHOD(constructor, "wkbSize", wkbSize);
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
  NODE_SET_PROTOTYPE_METHOD(constructor, "unionCascaded", unionCascaded);
  NODE_SET_PROTOTYPE_METHOD(constructor, "difference", difference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "symDifference", symDifference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "centroid", centroid);
  NODE_SET_PROTOTYPE_METHOD(constructor, "simplify", simplify);
  NODE_SET_PROTOTYPE_METHOD(constructor, "simplifyPreserveTopology", simplifyPreserveTopology);
  NODE_SET_PROTOTYPE_METHOD(constructor, "polygonize", polygonize);
  NODE_SET_PROTOTYPE_METHOD(constructor, "segmentize", segmentize);
  NODE_SET_PROTOTYPE_METHOD(constructor, "swapXY", swapXY);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getArea", getArea);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addPoint", addPoint);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addGeometry", addGeometry);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getNumGeometries", getNumGeometries);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getEnvelope", getEnvelope);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getEnvelope3D", getEnvelope3D);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getLength", getLength);
  NODE_SET_PROTOTYPE_METHOD(constructor, "value", value);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getPoint", getPoint);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getX", getX);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getY", getY);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getZ", getZ);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setX", setX);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setY", setY);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setZ", setZ);
  NODE_SET_PROTOTYPE_METHOD(constructor, "transform", transform);
  NODE_SET_PROTOTYPE_METHOD(constructor, "transformTo", transformTo);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getSpatialReference", getSpatialReference);
  NODE_SET_PROTOTYPE_METHOD(constructor, "assignSpatialReference", assignSpatialReference);


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
  Geometry* f;

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void* ptr = ext->Value();
    f = static_cast<Geometry *>(ptr);

  }else{
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

  //make a copy of geometry owned by a feature
  // + no need to track when a feature is destroyed
  // + no need to throw errors when a method trys to modify an owned read-only geometry
  // - is slower
  
  if(!owned) geom = geom->clone();

  Geometry *wrapped = new Geometry(geom);
  wrapped->owned_ = true;

  wrapped->size_ = geom->WkbSize();
  V8::AdjustAmountOfExternalAllocatedMemory(wrapped->size_);

  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = Geometry::constructor->GetFunction()->NewInstance(1, &ext);
  
  return scope.Close(obj);
}

Handle<Value> Geometry::toString(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  std::ostringstream ss;
  ss << "Geometry (" << geom->this_->getGeometryName() << ")";
  return scope.Close(String::New(ss.str().c_str()));
}

void Geometry::updateAmountOfExternalAllocatedMemory(Geometry* geom){
  int new_size = geom->this_->WkbSize();
  if(geom->owned_) V8::AdjustAmountOfExternalAllocatedMemory(new_size - geom->size_);
  geom->size_ = new_size;
}



OGRwkbGeometryType Geometry::getGeometryType_fixed(Geometry* geom)
{
  //For some reason OGRLinearRing::getGeometryType uses OGRLineString's method...
  //meaning OGRLinearRing::getGeometryType returns wkbLineString

  //http://trac.osgeo.org/gdal/ticket/1755

  OGRwkbGeometryType type =  wkbFlatten(geom->this_->getGeometryType());

  if(type == wkbLineString){
    if(std::string(geom->this_->getGeometryName()) == "LINEARRING"){
      return wkbLinearRing;
    }  }

  return type;
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


Handle<Value> Geometry::unionCascaded(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  if (Geometry::getGeometryType_fixed(geom) != wkbMultiPolygon) {
    return NODE_THROW("Geometry must be a MultiPolygon for a cascaded union.");
  }

  return scope.Close(Geometry::New(geom->this_->UnionCascaded()));
}


// The Centroid method wants the caller to create the point to fill in. Instead
// of requiring the caller to create the point geometry to fill in, we new up an
// OGRPoint and put the result into it and return that.
Handle<Value> Geometry::centroid(const Arguments& args)
{
  HandleScope scope;
  OGRPoint *point = new OGRPoint();

  ObjectWrap::Unwrap<Geometry>(args.This())->this_->Centroid(point);

  return scope.Close(Geometry::New(point));
}


Handle<Value> Geometry::polygonize(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  if (Geometry::getGeometryType_fixed(geom) != wkbMultiLineString) {
    return NODE_THROW("Geometry must be a wkbMultiLineString to polygonize.");
  }

  OGRGeometry *polygonized = geom->this_->Polygonize();

  if (polygonized != NULL) {
    return scope.Close(Geometry::New(polygonized));
  } else {
    return Null();
  }
}

Handle<Value> Geometry::getArea(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  Handle<Value> area = Undefined();

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbPolygon:
      area = Number::New(((OGRPolygon *)geom->this_)->get_Area());
      break;

    case wkbMultiPolygon:
      area = Number::New(((OGRMultiPolygon *)geom->this_)->get_Area());
      break;

    case wkbLinearRing:
      area = Number::New(((OGRLinearRing *)geom->this_)->get_Area());
      break;

    case wkbGeometryCollection:
      area = Number::New(((OGRGeometryCollection *)geom->this_)->get_Area());
      break;

    default:
      area = NODE_THROW("geometry must be either a polygon, multi-polygon, linear ring, or a geometry collection to use the area function.");
      break;
  }

  return scope.Close(area);
}

Handle<Value> Geometry::addPoint(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  OGRPoint *ogr_pt = NULL;
  double x = 0, y = 0, z = 0;
  bool is_3D = true;

  if(args[0]->IsNumber()){
    NODE_ARG_DOUBLE(0, "x", x);
    NODE_ARG_DOUBLE(1, "y", y);
    NODE_ARG_DOUBLE_OPT(2, "z", z);
    if(args.Length() < 3) is_3D = false;
  }else{
    Geometry* pt;
    NODE_ARG_WRAPPED(0, "point", Geometry, pt);
    ogr_pt = ((OGRPoint *)pt->this_);
  }

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbLinearRing:
      if(ogr_pt)       ((OGRLinearRing *)geom->this_)->addPoint(ogr_pt);
      else if(is_3D)   ((OGRLinearRing *)geom->this_)->addPoint(x, y, z);
      else             ((OGRLinearRing *)geom->this_)->addPoint(x, y);
      break;
    case wkbLineString:
      if(ogr_pt)       ((OGRLineString *)geom->this_)->addPoint(ogr_pt);
      else if(is_3D)   ((OGRLineString *)geom->this_)->addPoint(x, y, z);
      else             ((OGRLineString *)geom->this_)->addPoint(x, y);
      break;
    default:
      NODE_THROW("geometry must be either a line string or linear ring to use the addPoint function.");
      break;
  }

  Geometry::updateAmountOfExternalAllocatedMemory(geom);

  return Undefined();
}

Handle<Value> Geometry::addGeometry(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  Geometry *geom_insert;
  NODE_ARG_WRAPPED(0, "geometry", Geometry, geom_insert);

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbMultiPoint:         ((OGRMultiPoint *)geom->this_)->addGeometry(geom_insert->this_); break;
    case wkbMultiLineString:    ((OGRMultiLineString *)geom->this_)->addGeometry(geom_insert->this_); break;
    case wkbMultiPolygon:       ((OGRMultiPolygon *)geom->this_)->addGeometry(geom_insert->this_); break;
    case wkbGeometryCollection: ((OGRGeometryCollection *)geom->this_)->addGeometry(geom_insert->this_); break;
    case wkbPolygon:
      if(Geometry::getGeometryType_fixed(geom_insert) == wkbLinearRing){
        ((OGRPolygon *)geom->this_)->addRing((OGRLinearRing*) geom_insert->this_);
      }else{
        NODE_THROW("geometry must be linear ring to be added to a polygon.");
      }
      break;
    default:
      NODE_THROW("geometry must be either a geometry collection or polygon to use the addGeometry function.");
      break;
  }

  Geometry::updateAmountOfExternalAllocatedMemory(geom);

  return Undefined();
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

Handle<Value> Geometry::getLength(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  double length;

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbLinearRing:
      length = ((OGRLinearRing* )geom->this_)->get_Length();
    case wkbLineString:
      length = ((OGRLineString* )geom->this_)->get_Length();
    case wkbGeometryCollection:
      length = ((OGRGeometryCollection* )geom->this_)->get_Length();
    case wkbMultiLineString:
      length = ((OGRMultiLineString* )geom->this_)->get_Length();
    default:
      return NODE_THROW("geometry must be linear ring or line string to find length")
  }

  return scope.Close(Number::New(length));
}


Handle<Value> Geometry::value(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  OGRPoint *pt = new OGRPoint();
  double dist;

  NODE_ARG_DOUBLE(0, "distance", dist);

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbLinearRing:
      ((OGRLinearRing* )geom->this_)->Value(dist, pt);
    case wkbLineString:
      ((OGRLineString* )geom->this_)->Value(dist, pt);
    default:
      return NODE_THROW("geometry must be linear ring or line string to the value at specified distance")
  }

  return scope.Close(Geometry::New(pt));
}

Handle<Value> Geometry::getPoint(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  OGRPoint *pt = new OGRPoint();
  int i;

  NODE_ARG_INT(0, "i", i);

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbLinearRing:
      ((OGRLinearRing* )geom->this_)->getPoint(i, pt);
    case wkbLineString:
      ((OGRLineString* )geom->this_)->getPoint(i, pt);
    default:
      return NODE_THROW("geometry must be linear ring or line string to get point at index")
  }

  return scope.Close(Geometry::New(pt));
}


Handle<Value> Geometry::getX(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    return scope.Close(Number::New(((OGRPoint* )geom->this_)->getX()));
  } else {
    return NODE_THROW("geometry must be point to get x")
  }
}

Handle<Value> Geometry::getY(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    return scope.Close(Number::New(((OGRPoint* )geom->this_)->getY()));
  } else {
    return NODE_THROW("geometry must be point to get y")
  }
}

Handle<Value> Geometry::getZ(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    return scope.Close(Number::New(((OGRPoint* )geom->this_)->getZ()));
  } else {
    return NODE_THROW("geometry must be point to get z")
  }
}

Handle<Value> Geometry::setX(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  double x;
  NODE_ARG_DOUBLE(0, "x", x);

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    ((OGRPoint* )geom->this_)->setX(x);
  } else {
    return NODE_THROW("geometry must be point to set x")
  }
  return Undefined();
}

Handle<Value> Geometry::setY(const Arguments& args)
{
  HandleScope scope;
  
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  double y;
  NODE_ARG_DOUBLE(0, "y", y);

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    ((OGRPoint* )geom->this_)->setY(y);
  } else {
    return NODE_THROW("geometry must be point to set y")
  }
  return Undefined();
}

Handle<Value> Geometry::setZ(const Arguments& args)
{
  HandleScope scope;

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  double z;
  NODE_ARG_DOUBLE(0, "z", z);

  if(Geometry::getGeometryType_fixed(geom) == wkbPoint) {
    ((OGRPoint* )geom->this_)->setX(z);
  } else {
    return NODE_THROW("geometry must be point to set z")
  }
  return Undefined();
}

Handle<Value> Geometry::getSpatialReference(const Arguments& args)
{
  HandleScope scope;
  
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  return scope.Close(SpatialReference::New(geom->this_->getSpatialReference(), false));
}

Handle<Value> Geometry::getGeometry(const Arguments& args)
{
  HandleScope scope;

  int i;
  NODE_ARG_DOUBLE(0, "i", i);

  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
  OGRGeometry *subgeometry;


  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbMultiPoint:
      subgeometry = ((OGRMultiPoint *)geom->this_)->getGeometryRef(i); break;
    case wkbMultiLineString:
      subgeometry = ((OGRMultiLineString *)geom->this_)->getGeometryRef(i); break;
    case wkbMultiPolygon:
      subgeometry = ((OGRMultiPolygon *)geom->this_)->getGeometryRef(i); break;
    case wkbGeometryCollection:
      subgeometry = ((OGRGeometryCollection *)geom->this_)->getGeometryRef(i); break;
    default:
      return NODE_THROW("geometry must be geometry collection to use the getGeometry function.");
  }

  return scope.Close(Geometry::New(subgeometry, false));
}

Handle<Value> Geometry::getNumGeometries(const Arguments& args)
{
  HandleScope scope;
  Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

  switch (Geometry::getGeometryType_fixed(geom)) {
    case wkbMultiPoint:
      return scope.Close(Integer::New(((OGRMultiPoint *)geom->this_)->getNumGeometries()));
    case wkbMultiLineString:
      return scope.Close(Integer::New(((OGRMultiLineString *)geom->this_)->getNumGeometries()));
    case wkbMultiPolygon:
      return scope.Close(Integer::New(((OGRMultiPolygon *)geom->this_)->getNumGeometries()));
    case wkbGeometryCollection:
      return scope.Close(Integer::New(((OGRGeometryCollection *)geom->this_)->getNumGeometries()));
    default:
      return NODE_THROW("geometry must be geometry collection to use the getNumGeometries function.");
  }
}
