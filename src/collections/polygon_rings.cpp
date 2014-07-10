#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_polygon.hpp"
#include "../gdal_linearring.hpp"
#include "polygon_rings.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> PolygonRings::constructor;

void PolygonRings::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(PolygonRings::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("PolygonRings"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "add", add);

	target->Set(String::NewSymbol("PolygonRings"), constructor->GetFunction());
}

PolygonRings::PolygonRings()
	: ObjectWrap()
{}

PolygonRings::~PolygonRings() 
{}

Handle<Value> PolygonRings::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		PolygonRings *geom =  static_cast<PolygonRings *>(ptr);
		geom->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create PolygonRings directly");
	}
}

Handle<Value> PolygonRings::New(Handle<Value> geom)
{
	HandleScope scope;

	PolygonRings *wrapped = new PolygonRings();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = PolygonRings::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), geom);

	return scope.Close(obj);
}

Handle<Value> PolygonRings::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("PolygonRings"));
}

Handle<Value> PolygonRings::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);

	int i = geom->get()->getExteriorRing() ? 1 : 0;
	i += geom->get()->getNumInteriorRings();

	return scope.Close(Integer::New(i));
}

Handle<Value> PolygonRings::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);
	
	int i;
	NODE_ARG_INT(0, "index", i);

	if(i == 0) {
		return scope.Close(LinearRing::New(geom->get()->getExteriorRing(), false));
	} else {
		return scope.Close(LinearRing::New(geom->get()->getInteriorRing(i-1), false));
	}
}

Handle<Value> PolygonRings::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);
	
	LinearRing *ring;

	if (args.Length() < 1) {
		return NODE_THROW("ring(s) must be given");
	}
	Local<Object> obj = args[0]->ToObject();
	if (args[0]->IsArray()){
		//set from array of geometry objects
		Handle<Array> array = Handle<Array>::Cast(args[0]);
		int length = array->Length();
		for (int i = 0; i < length; i++){
			Handle<Object> element = array->Get(i)->ToObject();
			if(LinearRing::constructor->HasInstance(element)){
				ring = ObjectWrap::Unwrap<LinearRing>(obj);
				geom->get()->addRing(ring->get());
			} else {
				return NODE_THROW("All array elements must be LinearRings")
			}
		}
	} else if (LinearRing::constructor->HasInstance(obj)){
		ring = ObjectWrap::Unwrap<LinearRing>(obj);
		geom->get()->addRing(ring->get());
	} else {
		return NODE_THROW("ring(s) must be a LinearRing or array of LinearRings")
	}

	return Undefined();
}

} // namespace node_gdal