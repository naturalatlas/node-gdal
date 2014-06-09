
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"

#include <stdlib.h>

using namespace node_ogr;

Persistent<FunctionTemplate> GeometryCollection::constructor;

void GeometryCollection::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(GeometryCollection::New));
	constructor->Inherit(Geometry::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("GeometryCollection"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getArea", getArea);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getLength", getLength);
	NODE_SET_PROTOTYPE_METHOD(constructor, "addGeometry", addGeometry);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNumGeometries", getNumGeometries);

	target->Set(String::NewSymbol("GeometryCollection"), constructor->GetFunction());
}

GeometryCollection::GeometryCollection(OGRGeometryCollection *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{}

GeometryCollection::GeometryCollection()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


GeometryCollection::~GeometryCollection()
{
	if (owned_ && this_) {
		OGRGeometryFactory::destroyGeometry(this_);
		V8::AdjustAmountOfExternalAllocatedMemory(-size_);
	}
	this_ = NULL;
}

Handle<Value> GeometryCollection::New(const Arguments& args)
{
	HandleScope scope;
	GeometryCollection *f;

	if (!args.IsConstructCall()) {
		return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<GeometryCollection *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("GeometryCollection constructor doesn't take any arguments");
		}
		f = new GeometryCollection(new OGRGeometryCollection());
	}

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> GeometryCollection::New(OGRGeometryCollection *geom)
{
	HandleScope scope;
	return scope.Close(GeometryCollection::New(geom, true));
}

Handle<Value> GeometryCollection::New(OGRGeometryCollection *geom, bool owned)
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
		geom = static_cast<OGRGeometryCollection*>(geom->clone());
	};

	GeometryCollection *wrapped = new GeometryCollection(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = GeometryCollection::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}


Handle<Value> GeometryCollection::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("GeometryCollection"));
}


NODE_WRAPPED_METHOD_WITH_RESULT(GeometryCollection, getArea, Number, get_Area);
NODE_WRAPPED_METHOD_WITH_RESULT(GeometryCollection, getLength, Number, get_Length);
NODE_WRAPPED_METHOD_WITH_RESULT(GeometryCollection, getNumGeometries, Integer, getNumGeometries);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(GeometryCollection, addGeometry, addGeometry, Geometry, "geometry to add");

Handle<Value> GeometryCollection::getGeometry(const Arguments& args)
{
	HandleScope scope;
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(args.This());

	int i;
	NODE_ARG_INT(0, "index", i);

	return scope.Close(Geometry::New(geom->this_->getGeometryRef(i), false));
}