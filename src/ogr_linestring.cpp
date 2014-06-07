
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"

#include <stdlib.h>

using namespace node_ogr;

Persistent<FunctionTemplate> LineString::constructor;

void LineString::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LineString::New));
  constructor->Inherit(Geometry::constructor);
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("LineString"));

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addPoint", addPoint);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getPoint", getPoint);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getNumPoints", getNumPoints);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getLength", getLength);
  NODE_SET_PROTOTYPE_METHOD(constructor, "value", value);


  target->Set(String::NewSymbol("LineString"), constructor->GetFunction());
}

LineString::LineString(OGRLineString *geom)
: ObjectWrap(),
  this_(geom),
  owned_(true),
  size_(0)
{}

LineString::LineString()
: ObjectWrap(),
  this_(NULL),
  owned_(true),
  size_(0)
{
}


LineString::~LineString()
{
  if (owned_ && this_) {
    OGRGeometryFactory::destroyGeometry(this_);
    V8::AdjustAmountOfExternalAllocatedMemory(-size_);
  }
  this_ = NULL;
}

Handle<Value> LineString::New(const Arguments& args)
{
  HandleScope scope;
  LineString *f;

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void* ptr = ext->Value();
    f = static_cast<LineString *>(ptr);

  }else{
    if(args.Length() != 0)
      return NODE_THROW("LineString constructor doesn't take any arguments"); 
    f = new LineString(new OGRLineString());
  }

  f->Wrap(args.This());
  return args.This();
}

Handle<Value> LineString::New(OGRLineString *geom) {
  HandleScope scope;
  return scope.Close(LineString::New(geom, true));
}

Handle<Value> LineString::New(OGRLineString *geom, bool owned) {
  HandleScope scope;

  if(!geom) return Null();

  //make a copy of geometry owned by a feature
  // + no need to track when a feature is destroyed
  // + no need to throw errors when a method trys to modify an owned read-only geometry
  // - is slower
  
  if(!owned) geom = static_cast<OGRLineString*>(geom->clone());;

  LineString *wrapped = new LineString(geom);
  wrapped->owned_ = true;

  UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = LineString::constructor->GetFunction()->NewInstance(1, &ext);
  
  return scope.Close(obj);
}

Handle<Value> LineString::toString(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(String::New("LineString"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(LineString, getLength, Number, get_Length);
NODE_WRAPPED_METHOD_WITH_RESULT(LineString, getNumPoints, Integer, getNumPoints);

Handle<Value> LineString::getPoint(const Arguments& args)
{
  HandleScope scope;

  LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

  OGRPoint *pt = new OGRPoint();
  int i;

  NODE_ARG_INT(0, "i", i);

  geom->this_->getPoint(i, pt);

  return scope.Close(Point::New(pt));
}

Handle<Value> LineString::value(const Arguments& args)
{
  HandleScope scope;

  LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

  OGRPoint *pt = new OGRPoint();
  double dist;

  NODE_ARG_DOUBLE(0, "distance", dist);

  geom->this_->Value(dist, pt);

  return scope.Close(Point::New(pt));
}


Handle<Value> LineString::addPoint(const Arguments& args)
{
  HandleScope scope;

  LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

  if(args[0]->IsNumber()){

    double x = 0, y = 0, z = 0;
    NODE_ARG_DOUBLE(0, "x", x);
    NODE_ARG_DOUBLE(1, "y", y);
    NODE_ARG_DOUBLE_OPT(2, "z", z);
    if(args.Length() < 3) geom->this_->addPoint(x, y);
    else geom->this_->addPoint(x, y, z);

  }else{

    Point* pt;
    NODE_ARG_WRAPPED(0, "point", Point, pt);
    geom->this_->addPoint(pt->get());

  }

  return Undefined();
}