#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_geometrycollection.hpp"
#include "geometry_collection_children.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> GeometryCollectionChildren::constructor;

void GeometryCollectionChildren::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(GeometryCollectionChildren::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("GeometryCollectionChildren"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(constructor, "add", add);

	target->Set(String::NewSymbol("GeometryCollectionChildren"), constructor->GetFunction());
}

GeometryCollectionChildren::GeometryCollectionChildren()
	: ObjectWrap()
{}

GeometryCollectionChildren::~GeometryCollectionChildren() 
{}

Handle<Value> GeometryCollectionChildren::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		GeometryCollectionChildren *geom =  static_cast<GeometryCollectionChildren *>(ptr);
		geom->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create GeometryCollectionChildren directly");
	}
}

Handle<Value> GeometryCollectionChildren::New(Handle<Value> geom)
{
	HandleScope scope;

	GeometryCollectionChildren *wrapped = new GeometryCollectionChildren();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = GeometryCollectionChildren::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), geom);

	return scope.Close(obj);
}

Handle<Value> GeometryCollectionChildren::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("GeometryCollectionChildren"));
}

Handle<Value> GeometryCollectionChildren::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	return scope.Close(Integer::New(geom->get()->getNumGeometries()));
}

Handle<Value> GeometryCollectionChildren::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);
	
	int i;
	NODE_ARG_INT(0, "index", i);

	return scope.Close(Geometry::New(geom->get()->getGeometryRef(i), false));
}

Handle<Value> GeometryCollectionChildren::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);

	OGRErr err = geom->get()->removeGeometry(i);
	if (err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> GeometryCollectionChildren::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);
	
	Geometry *child;

	if (args.Length() < 1) {
		return NODE_THROW("child(ren) must be given");
	}
	Local<Object> obj = args[0]->ToObject();
	if (args[0]->IsArray()){
		//set from array of geometry objects
		Handle<Array> array = Handle<Array>::Cast(args[0]);
		int length = array->Length();
		for (int i = 0; i < length; i++){
			Handle<Object> element = array->Get(i)->ToObject();
			if(Geometry::constructor->HasInstance(element)){
				child = ObjectWrap::Unwrap<Geometry>(obj);
				OGRErr err = geom->get()->addGeometry(child->get());
				if (err) {
					return NODE_THROW_OGRERR(err);
				}
			} else {
				return NODE_THROW("All array elements must be geometry object")
			}
		}
	} else if (Geometry::constructor->HasInstance(obj)){
		child = ObjectWrap::Unwrap<Geometry>(obj);
		OGRErr err = geom->get()->addGeometry(child->get());
		if (err) {
			return NODE_THROW_OGRERR(err);
		}
	} else {
		return NODE_THROW("child must be a geometry object or array of geometry objects")
	}

	return Undefined();
}

} // namespace node_gdal