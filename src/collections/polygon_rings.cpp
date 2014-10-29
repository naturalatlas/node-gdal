#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_polygon.hpp"
#include "../gdal_linearring.hpp"
#include "polygon_rings.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> PolygonRings::constructor;

void PolygonRings::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(PolygonRings::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("PolygonRings"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);

	target->Set(NanNew("PolygonRings"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

PolygonRings::PolygonRings()
	: ObjectWrap()
{}

PolygonRings::~PolygonRings()
{}

NAN_METHOD(PolygonRings::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		PolygonRings *geom =  static_cast<PolygonRings *>(ptr);
		geom->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create PolygonRings directly");
		NanReturnUndefined();
	}
}

Handle<Value> PolygonRings::New(Handle<Value> geom)
{
	NanEscapableScope();

	PolygonRings *wrapped = new PolygonRings();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(PolygonRings::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), geom);

	return NanEscapeScope(obj);
}

NAN_METHOD(PolygonRings::toString)
{
	NanScope();
	NanReturnValue(NanNew("PolygonRings"));
}

NAN_METHOD(PolygonRings::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);

	int i = geom->get()->getExteriorRing() ? 1 : 0;
	i += geom->get()->getNumInteriorRings();

	NanReturnValue(NanNew<Integer>(i));
}

NAN_METHOD(PolygonRings::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);

	if(i == 0) {
		NanReturnValue(LinearRing::New(geom->get()->getExteriorRing(), false));
	} else {
		NanReturnValue(LinearRing::New(geom->get()->getInteriorRing(i-1), false));
	}
}

NAN_METHOD(PolygonRings::add)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Polygon *geom = ObjectWrap::Unwrap<Polygon>(parent);

	LinearRing *ring;

	if (args.Length() < 1) {
		NanThrowError("ring(s) must be given");
		NanReturnUndefined();
	}
	if (args[0]->IsArray()){
		//set from array of geometry objects
		Handle<Array> array = args[0].As<Array>();
		int length = array->Length();
		for (int i = 0; i < length; i++){
			Handle<Value> element = array->Get(i);
			if (IS_WRAPPED(element, LinearRing)){
				ring = ObjectWrap::Unwrap<LinearRing>(element.As<Object>());
				geom->get()->addRing(ring->get());
			} else {
				NanThrowError("All array elements must be LinearRings");
				NanReturnUndefined();
			}
		}
	} else if (IS_WRAPPED(args[0], LinearRing)){
		ring = ObjectWrap::Unwrap<LinearRing>(args[0].As<Object>());
		geom->get()->addRing(ring->get());
	} else {
		NanThrowError("ring(s) must be a LinearRing or array of LinearRings");
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

} // namespace node_gdal