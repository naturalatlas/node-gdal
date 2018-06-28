
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_polygon.hpp"
#include "collections/polygon_rings.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Polygon::constructor;

void Polygon::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Polygon::New);
	lcons->Inherit(Nan::New(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Polygon").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getArea", getArea);

	ATTR(lcons, "rings", ringsGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("Polygon").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

Polygon::Polygon(OGRPolygon *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created Polygon [%p]", geom);
}

Polygon::Polygon()
	: Nan::ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


Polygon::~Polygon()
{
	if(this_) {
		LOG("Disposing Polygon [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			Nan::AdjustExternalMemory(-size_);
		}
		LOG("Disposed Polygon [%p]", this_);
		this_ = NULL;
	}
}

/**
 * Concrete class representing polygons.
 *
 * @constructor
 * @class gdal.Polygon
 * @extends gdal.Geometry
 */
NAN_METHOD(Polygon::New)
{
	Nan::HandleScope scope;
	Polygon *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Polygon *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("Polygon constructor doesn't take any arguments");
			return;
		}
		f = new Polygon(new OGRPolygon());
	}

	Local<Value> rings = PolygonRings::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("rings_").ToLocalChecked(), rings);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> Polygon::New(OGRPolygon *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(Polygon::New(geom, true));
}

Local<Value> Polygon::New(OGRPolygon *geom, bool owned)
{
	Nan::EscapableHandleScope scope;

	if (!geom) {
		return scope.Escape(Nan::Null());
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

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Polygon::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(Polygon::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("Polygon").ToLocalChecked());
}

/**
 * Computes the area of the polygon.
 *
 * @method getArea
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(Polygon, getArea, Number, get_Area);

/**
 * The rings that make up the polygon geometry.
 *
 * @attribute rings
 * @type {gdal.PolygonRings}
 */
NAN_GETTER(Polygon::ringsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("rings_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
