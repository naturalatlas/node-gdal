
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_linearring.hpp"
#include "gdal_linestring.hpp"
#include "collections/linestring_points.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> LinearRing::constructor;

void LinearRing::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(LinearRing::New);
	lcons->Inherit(Nan::New(LineString::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("LinearRing").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getArea", getArea);

	target->Set(Nan::New("LinearRing").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

LinearRing::LinearRing(OGRLinearRing *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created LinearRing [%p]", geom);
}

LinearRing::LinearRing()
	: Nan::ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


LinearRing::~LinearRing()
{
	if(this_) {
		LOG("Disposing LinearRing [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			Nan::AdjustExternalMemory(-size_);
		}
		LOG("Disposed LinearRing [%p]", this_);
		this_ = NULL;
	}
}

/**
 * Concrete representation of a closed ring.
 *
 * @constructor
 * @class gdal.LinearRing
 * @extends gdal.LineString
 */
NAN_METHOD(LinearRing::New)
{
	Nan::HandleScope scope;
	LinearRing *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<LinearRing *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("LinearRing constructor doesn't take any arguments");
			return;
		}
		f = new LinearRing(new OGRLinearRing());
	}

	Local<Value> points = LineStringPoints::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("points_").ToLocalChecked(), points);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> LinearRing::New(OGRLinearRing *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(LinearRing::New(geom, true));
}

Local<Value> LinearRing::New(OGRLinearRing *geom, bool owned)
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
		geom = static_cast<OGRLinearRing*>(geom->clone());
	};

	LinearRing *wrapped = new LinearRing(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(LinearRing::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(LinearRing::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("LinearRing").ToLocalChecked());
}

/**
 * Computes the area enclosed by the ring.
 *
 * @method getArea
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(LinearRing, getArea, Number, get_Area);

} // namespace node_gdal