#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_point.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> Point::constructor;

void Point::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Point::New));
	constructor->Inherit(Geometry::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Point"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);

	// properties
	ATTR(constructor, "x", xGetter, xSetter);
	ATTR(constructor, "y", yGetter, ySetter);
	ATTR(constructor, "z", zGetter, zSetter);

	target->Set(String::NewSymbol("Point"), constructor->GetFunction());
}

Point::Point(OGRPoint *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created Point [%p]", geom);
}

Point::Point()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


Point::~Point()
{
	if(this_) {
		LOG("Disposing Point [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			V8::AdjustAmountOfExternalAllocatedMemory(-size_);
		}
		LOG("Disposed Point [%p]", this_);
		this_ = NULL;
	}
}

Handle<Value> Point::New(const Arguments& args)
{
	HandleScope scope;
	Point *f;
	OGRPoint *geom;
	double x = 0, y = 0, z = 0;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<Point *>(ptr);

	} else {
		NODE_ARG_DOUBLE_OPT(0, "x", x);
		NODE_ARG_DOUBLE_OPT(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);

		if (args.Length() == 1) {
			return NODE_THROW("Point constructor must be given 0, 2, or 3 arguments");
		}

		if (args.Length() == 3) {
			geom = new OGRPoint(x, y, z);
		} else {
			geom = new OGRPoint(x, y);
		}

		f = new Point(geom);
	}

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> Point::New(OGRPoint *geom)
{
	HandleScope scope;
	return scope.Close(Point::New(geom, true));
}

Handle<Value> Point::New(OGRPoint *geom, bool owned)
{
	HandleScope scope;

	if (!geom) {
		return Null();
	}

	//make a copy of geometry owned by a feature
	// + no need to track when a feature is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only geometry
	// - is slower

	if (!owned) {
		geom = static_cast<OGRPoint*>(geom->clone());
	}

	Point *wrapped = new Point(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Point::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> Point::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Point"));
}

Handle<Value> Point::xGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());
	return scope.Close(Number::New((geom->this_)->getX()));
}

void Point::xSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		NODE_THROW("y must be a number");
		return;
	}
	double x = value->NumberValue();

	((OGRPoint* )geom->this_)->setX(x);
}

Handle<Value> Point::yGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());
	return scope.Close(Number::New((geom->this_)->getY()));
}

void Point::ySetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		NODE_THROW("y must be a number");
		return;
	}
	double y = value->NumberValue();

	((OGRPoint* )geom->this_)->setY(y);
}

Handle<Value> Point::zGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());
	return scope.Close(Number::New((geom->this_)->getZ()));
}

void Point::zSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	Point *geom = ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		NODE_THROW("z must be a number");
		return;
	}
	double z = value->NumberValue();

	((OGRPoint* )geom->this_)->setZ(z);
}

} // namespace node_gdal