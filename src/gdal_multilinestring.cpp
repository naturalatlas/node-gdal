
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multilinestring.hpp"
#include "gdal_linestring.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MultiLineString::constructor;

void MultiLineString::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(MultiLineString::New));
	constructor->Inherit(GeometryCollection::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("MultiLineString"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "polygonize", polygonize);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);

	target->Set(String::NewSymbol("MultiLineString"), constructor->GetFunction());
}

MultiLineString::MultiLineString(OGRMultiLineString *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiLineString [%p]", geom);
}

MultiLineString::MultiLineString()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


MultiLineString::~MultiLineString()
{
	if(this_) {
		LOG("Disposing GeometryCollection [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			V8::AdjustAmountOfExternalAllocatedMemory(-size_);
		}
		LOG("Disposed GeometryCollection [%p]", this_);
		this_ = NULL;
	}
}

Handle<Value> MultiLineString::New(const Arguments& args)
{
	HandleScope scope;
	MultiLineString *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<MultiLineString *>(ptr);

	} else {
		if (args.Length() != 0) {
			return NODE_THROW("MultiLineString constructor doesn't take any arguments");
		}
		f = new MultiLineString(new OGRMultiLineString());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("children_"), children); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> MultiLineString::New(OGRMultiLineString *geom)
{
	HandleScope scope;
	return scope.Close(MultiLineString::New(geom, true));
}

Handle<Value> MultiLineString::New(OGRMultiLineString *geom, bool owned)
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
		geom = static_cast<OGRMultiLineString*>(geom->clone());
	};

	MultiLineString *wrapped = new MultiLineString(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = MultiLineString::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> MultiLineString::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("MultiLineString"));
}


NODE_WRAPPED_METHOD_WITH_RESULT(MultiLineString, polygonize, Geometry, Polygonize);

Handle<Value> MultiLineString::getGeometry(const Arguments& args)
{
	HandleScope scope;
	MultiLineString *geom = ObjectWrap::Unwrap<MultiLineString>(args.This());

	int i;
	NODE_ARG_INT(0, "index", i);

	return scope.Close(LineString::New(static_cast<OGRLineString*>(geom->this_->getGeometryRef(i)), false));
}

} // namespace node_gdal