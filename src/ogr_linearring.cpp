
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"
#include "collections/linestring_points.hpp"

#include <stdlib.h>

using namespace node_ogr;

Persistent<FunctionTemplate> LinearRing::constructor;

void LinearRing::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LinearRing::New));
	constructor->Inherit(LineString::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LinearRing"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getArea", getArea);

	target->Set(String::NewSymbol("LinearRing"), constructor->GetFunction());
}

LinearRing::LinearRing(OGRLinearRing *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{}

LinearRing::LinearRing()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


LinearRing::~LinearRing()
{
	if (owned_ && this_) {
		OGRGeometryFactory::destroyGeometry(this_);
		V8::AdjustAmountOfExternalAllocatedMemory(-size_);
	}
	this_ = NULL;
}

Handle<Value> LinearRing::New(const Arguments& args)
{
	HandleScope scope;
	LinearRing *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<LinearRing *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("LinearRing constructor doesn't take any arguments");
		}
		f = new LinearRing(new OGRLinearRing());
	}

	Handle<Value> points = LineStringPoints::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("points_"), points); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> LinearRing::New(OGRLinearRing *geom)
{
	HandleScope scope;
	return scope.Close(LinearRing::New(geom, true));
}

Handle<Value> LinearRing::New(OGRLinearRing *geom, bool owned)
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
		geom = static_cast<OGRLinearRing*>(geom->clone());
	};

	LinearRing *wrapped = new LinearRing(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LinearRing::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> LinearRing::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LinearRing"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(LinearRing, getArea, Number, get_Area);