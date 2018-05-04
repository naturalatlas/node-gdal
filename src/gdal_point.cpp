#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_point.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Point::constructor;

void Point::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Point::New);
	lcons->Inherit(Nan::New(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Point").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);

	// properties
	ATTR(lcons, "x", xGetter, xSetter);
	ATTR(lcons, "y", yGetter, ySetter);
	ATTR(lcons, "z", zGetter, zSetter);

	target->Set(Nan::New("Point").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

Point::Point(OGRPoint *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created Point [%p]", geom);
}

Point::Point()
	: Nan::ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


Point::~Point()
{
	if(this_) {
		LOG("Disposing Point [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			Nan::AdjustExternalMemory(-size_);
		}
		LOG("Disposed Point [%p]", this_);
		this_ = NULL;
	}
}

/**
 * Point class.
 *
 * @constructor
 * @class gdal.Point
 * @extends gdal.Geometry
 * @param {Number} x
 * @param {Number} y
 * @param {Number} [z]
 */
NAN_METHOD(Point::New)
{
	Nan::HandleScope scope;
	Point *f;
	OGRPoint *geom;
	double x = 0, y = 0, z = 0;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Point *>(ptr);

	} else {
		NODE_ARG_DOUBLE_OPT(0, "x", x);
		NODE_ARG_DOUBLE_OPT(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);

		if (info.Length() == 1) {
			Nan::ThrowError("Point constructor must be given 0, 2, or 3 arguments");
			return;
		}

		if (info.Length() == 3) {
			geom = new OGRPoint(x, y, z);
		} else {
			geom = new OGRPoint(x, y);
		}

		f = new Point(geom);
	}

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> Point::New(OGRPoint *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(Point::New(geom, true));
}

Local<Value> Point::New(OGRPoint *geom, bool owned)
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
		geom = static_cast<OGRPoint*>(geom->clone());
	}

	Point *wrapped = new Point(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Point::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(Point::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("Point").ToLocalChecked());
}

/**
 * @attribute x
 * @type Number
 */
NAN_GETTER(Point::xGetter)
{
	Nan::HandleScope scope;
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());
	info.GetReturnValue().Set(Nan::New<Number>((geom->this_)->getX()));
}

NAN_SETTER(Point::xSetter)
{
	Nan::HandleScope scope;
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		Nan::ThrowError("y must be a number");
		return;
	}
	double x = value->NumberValue();

	((OGRPoint* )geom->this_)->setX(x);
}

/**
 * @attribute y
 * @type Number
 */
NAN_GETTER(Point::yGetter)
{
	Nan::HandleScope scope;
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());
	info.GetReturnValue().Set(Nan::New<Number>((geom->this_)->getY()));
}

NAN_SETTER(Point::ySetter)
{
	Nan::HandleScope scope;
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		Nan::ThrowError("y must be a number");
		return;
	}
	double y = value->NumberValue();

	((OGRPoint* )geom->this_)->setY(y);
}

/**
 * @attribute z
 * @type Number
 */
NAN_GETTER(Point::zGetter)
{
	Nan::HandleScope scope;
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());
	info.GetReturnValue().Set(Nan::New<Number>((geom->this_)->getZ()));
}

NAN_SETTER(Point::zSetter)
{
	Point *geom = Nan::ObjectWrap::Unwrap<Point>(info.This());

	if (!value->IsNumber()) {
		Nan::ThrowError("z must be a number");
		return;
	}
	double z = value->NumberValue();

	((OGRPoint* )geom->this_)->setZ(z);
}

} // namespace node_gdal