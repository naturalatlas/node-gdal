
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipoint.hpp"
#include "gdal_point.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MultiPoint::constructor;

void MultiPoint::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(MultiPoint::New));
	constructor->Inherit(GeometryCollection::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("MultiPoint"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);

	target->Set(String::NewSymbol("MultiPoint"), constructor->GetFunction());
}

MultiPoint::MultiPoint(OGRMultiPoint *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{}

MultiPoint::MultiPoint()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


MultiPoint::~MultiPoint()
{
	if (owned_ && this_) {
		OGRGeometryFactory::destroyGeometry(this_);
		V8::AdjustAmountOfExternalAllocatedMemory(-size_);
	}
	this_ = NULL;
}

Handle<Value> MultiPoint::New(const Arguments& args)
{
	HandleScope scope;
	MultiPoint *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<MultiPoint *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("MultiPoint constructor doesn't take any arguments");
		}
		f = new MultiPoint(new OGRMultiPoint());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("children_"), children); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> MultiPoint::New(OGRMultiPoint *geom)
{
	HandleScope scope;
	return scope.Close(MultiPoint::New(geom, true));
}

Handle<Value> MultiPoint::New(OGRMultiPoint *geom, bool owned)
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
		geom = static_cast<OGRMultiPoint*>(geom->clone());
	}

	MultiPoint *wrapped = new MultiPoint(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = MultiPoint::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> MultiPoint::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("MultiPoint"));
}

Handle<Value> MultiPoint::getGeometry(const Arguments& args)
{
	HandleScope scope;
	MultiPoint *geom = ObjectWrap::Unwrap<MultiPoint>(args.This());

	int i;
	NODE_ARG_INT(0, "index", i);

	return scope.Close(Point::New(static_cast<OGRPoint*>(geom->this_->getGeometryRef(i)), false));
}

} // namespace node_gdal