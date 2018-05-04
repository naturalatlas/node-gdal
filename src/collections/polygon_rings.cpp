#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_polygon.hpp"
#include "../gdal_linearring.hpp"
#include "polygon_rings.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> PolygonRings::constructor;

void PolygonRings::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(PolygonRings::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("PolygonRings").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "add", add);

	target->Set(Nan::New("PolygonRings").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

PolygonRings::PolygonRings()
	: Nan::ObjectWrap()
{}

PolygonRings::~PolygonRings()
{}

/**
 * A collection of polygon rings, used by {{#crossLink "gdal.Polygon"}}gdal.Polygon{{/crossLink}}.
 *
 * @class gdal.PolygonRings
 */
NAN_METHOD(PolygonRings::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		PolygonRings *geom =  static_cast<PolygonRings *>(ptr);
		geom->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create PolygonRings directly");
		return;
	}
}

Local<Value> PolygonRings::New(Local<Value> geom)
{
	Nan::EscapableHandleScope scope;

	PolygonRings *wrapped = new PolygonRings();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(PolygonRings::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), geom);

	return scope.Escape(obj);
}

NAN_METHOD(PolygonRings::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("PolygonRings").ToLocalChecked());
}

/**
 * Returns the number of rings that exist in the collection.
 *
 * @method count
 * @return Integer
 */
NAN_METHOD(PolygonRings::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Polygon *geom = Nan::ObjectWrap::Unwrap<Polygon>(parent);

	int i = geom->get()->getExteriorRing() ? 1 : 0;
	i += geom->get()->getNumInteriorRings();

	info.GetReturnValue().Set(Nan::New<Integer>(i));
}

/**
 * Returns the ring at the specified index. The ring
 * at index `0` will always be the polygon's exterior ring.
 *
 * @example
 * ```
 * var exterior = polygon.rings.get(0);
 * var interior = polygon.rings.get(1);```
 *
 * @method get
 * @return {gdal.LinearRing}
 */
NAN_METHOD(PolygonRings::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Polygon *geom = Nan::ObjectWrap::Unwrap<Polygon>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);

	if(i == 0) {
		info.GetReturnValue().Set(LinearRing::New(geom->get()->getExteriorRing(), false));
		return;
	} else {
		info.GetReturnValue().Set(LinearRing::New(geom->get()->getInteriorRing(i-1), false));
		return;
	}
}

/**
 * Adds a ring to the collection.
 *
 * @example
 * ```
 * var ring1 = new gdal.LinearRing();
 * ring1.points.add(0,0);
 * ring1.points.add(1,0);
 * ring1.points.add(1,1);
 * ring1.points.add(0,1);
 * ring1.points.add(0,0);
 *
 * // one at a time:
 * polygon.rings.add(ring1);
 *
 * // many at once:
 * polygon.rings.add([ring1, ...]);```
 *
 * @method add
 * @param {gdal.LinearRing} ring(s)
 */
NAN_METHOD(PolygonRings::add)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Polygon *geom = Nan::ObjectWrap::Unwrap<Polygon>(parent);

	LinearRing *ring;

	if (info.Length() < 1) {
		Nan::ThrowError("ring(s) must be given");
		return;
	}
	if (info[0]->IsArray()){
		//set from array of geometry objects
		Local<Array> array = info[0].As<Array>();
		int length = array->Length();
		for (int i = 0; i < length; i++){
			Local<Value> element = array->Get(i);
			if (IS_WRAPPED(element, LinearRing)){
				ring = Nan::ObjectWrap::Unwrap<LinearRing>(element.As<Object>());
				geom->get()->addRing(ring->get());
			} else {
				Nan::ThrowError("All array elements must be LinearRings");
				return;
			}
		}
	} else if (IS_WRAPPED(info[0], LinearRing)){
		ring = Nan::ObjectWrap::Unwrap<LinearRing>(info[0].As<Object>());
		geom->get()->addRing(ring->get());
	} else {
		Nan::ThrowError("ring(s) must be a LinearRing or array of LinearRings");
		return;
	}

	return;
}

} // namespace node_gdal