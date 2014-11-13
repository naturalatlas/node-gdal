#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_point.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> Point::constructor;

void Point::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(Point::New);
	lcons->Inherit(NanNew(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("Point"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);

	// properties
	ATTR(lcons, "x", xGetter, xSetter);
	ATTR(lcons, "y", yGetter, ySetter);
	ATTR(lcons, "z", zGetter, zSetter);

	target->Set(NanNew("Point"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

Point::Point(OGRPoint *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created Point [%p]", geom);
}

Point::Point()
	: ObjectWrap(),
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
			NanAdjustExternalMemory(-size_);
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
	NanScope();
	Point *f;
	OGRPoint *geom;
	double x = 0, y = 0, z = 0;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Point *>(ptr);

	} else {
		NODE_ARG_DOUBLE_OPT(0, "x", x);
		NODE_ARG_DOUBLE_OPT(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);

		if (args.Length() == 1) {
			NanThrowError("Point constructor must be given 0, 2, or 3 arguments");
			NanReturnUndefined();
		}

		if (args.Length() == 3) {
			geom = new OGRPoint(x, y, z);
		} else {
			geom = new OGRPoint(x, y);
		}

		f = new Point(geom);
	}

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> Point::New(OGRPoint *geom)
{
	NanEscapableScope();
	return NanEscapeScope(Point::New(geom, true));
}

Handle<Value> Point::New(OGRPoint *geom, bool owned)
{
	NanEscapableScope();

	if (!geom) {
		return NanEscapeScope(NanNull());
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

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Point::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(Point::toString)
{
	NanScope();
	NanReturnValue(NanNew("Point"));
}

/**
 * @attribute x
 * @type Number
 */
NAN_GETTER(Point::xGetter)
{
	NanScope();
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());
	NanReturnValue(NanNew<Number>((geom->this_)->getX()));
}

NAN_SETTER(Point::xSetter)
{
	NanScope();
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());

	if (!value->IsNumber()) {
		NanThrowError("y must be a number");
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
	NanScope();
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());
	NanReturnValue(NanNew<Number>((geom->this_)->getY()));
}

NAN_SETTER(Point::ySetter)
{
	NanScope();
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());

	if (!value->IsNumber()) {
		NanThrowError("y must be a number");
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
	NanScope();
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());
	NanReturnValue(NanNew<Number>((geom->this_)->getZ()));
}

NAN_SETTER(Point::zSetter)
{
	Point *geom = ObjectWrap::Unwrap<Point>(args.This());

	if (!value->IsNumber()) {
		NanThrowError("z must be a number");
		return;
	}
	double z = value->NumberValue();

	((OGRPoint* )geom->this_)->setZ(z);
}

} // namespace node_gdal