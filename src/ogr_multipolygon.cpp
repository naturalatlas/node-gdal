
#include "ogr_common.hpp"
#include "ogr_geometry.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

using namespace node_ogr;

Persistent<FunctionTemplate> MultiPolygon::constructor;

void MultiPolygon::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(MultiPolygon::New));
	constructor->Inherit(GeometryCollection::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("MultiPolygon"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "unionCascaded", unionCascaded);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getArea", getArea);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);

	target->Set(String::NewSymbol("MultiPolygon"), constructor->GetFunction());
}

MultiPolygon::MultiPolygon(OGRMultiPolygon *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{}

MultiPolygon::MultiPolygon()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


MultiPolygon::~MultiPolygon()
{
	if (owned_ && this_) {
		OGRGeometryFactory::destroyGeometry(this_);
		V8::AdjustAmountOfExternalAllocatedMemory(-size_);
	}
	this_ = NULL;
}

Handle<Value> MultiPolygon::New(const Arguments& args)
{
	HandleScope scope;
	MultiPolygon *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<MultiPolygon *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("MultiPolygon constructor doesn't take any arguments");
		}
		f = new MultiPolygon(new OGRMultiPolygon());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("children_"), children); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> MultiPolygon::New(OGRMultiPolygon *geom)
{
	HandleScope scope;
	return scope.Close(MultiPolygon::New(geom, true));
}

Handle<Value> MultiPolygon::New(OGRMultiPolygon *geom, bool owned)
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
		geom = static_cast<OGRMultiPolygon*>(geom->clone());
	};

	MultiPolygon *wrapped = new MultiPolygon(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = MultiPolygon::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> MultiPolygon::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("MultiPolygon"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(MultiPolygon, unionCascaded, Geometry, UnionCascaded);
NODE_WRAPPED_METHOD_WITH_RESULT(MultiPolygon, getArea, Number, get_Area);

Handle<Value> MultiPolygon::getGeometry(const Arguments& args)
{
	HandleScope scope;
	MultiPolygon *geom = ObjectWrap::Unwrap<MultiPolygon>(args.This());

	int i;
	NODE_ARG_INT(0, "index", i);

	return scope.Close(Polygon::New(static_cast<OGRPolygon*>(geom->this_->getGeometryRef(i)), false));
}