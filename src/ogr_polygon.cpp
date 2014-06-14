
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"
#include "collections/polygon_rings.hpp"

#include <stdlib.h>

using namespace node_ogr;

Persistent<FunctionTemplate> Polygon::constructor;

void Polygon::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Polygon::New));
	constructor->Inherit(Geometry::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Polygon"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getArea", getArea);

	ATTR(constructor, "rings", ringsGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("Polygon"), constructor->GetFunction());
}

Polygon::Polygon(OGRPolygon *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{}

Polygon::Polygon()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


Polygon::~Polygon()
{
	if (owned_ && this_) {
		OGRGeometryFactory::destroyGeometry(this_);
		V8::AdjustAmountOfExternalAllocatedMemory(-size_);
	}
	this_ = NULL;
}

Handle<Value> Polygon::New(const Arguments& args)
{
	HandleScope scope;
	Polygon *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<Polygon *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("Polygon constructor doesn't take any arguments");
		}
		f = new Polygon(new OGRPolygon());
	}

	Handle<Value> rings = PolygonRings::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("rings_"), rings); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> Polygon::New(OGRPolygon *geom)
{
	HandleScope scope;
	return scope.Close(Polygon::New(geom, true));
}

Handle<Value> Polygon::New(OGRPolygon *geom, bool owned)
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
		geom = static_cast<OGRPolygon*>(geom->clone());
	}

	Polygon *wrapped = new Polygon(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Polygon::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> Polygon::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Polygon"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(Polygon, getArea, Number, get_Area);

Handle<Value> Polygon::ringsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("rings_")));
}