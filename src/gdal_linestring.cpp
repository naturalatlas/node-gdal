
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_linestring.hpp"
#include "gdal_point.hpp"
#include "collections/linestring_points.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> LineString::constructor;

void LineString::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(LineString::New);
	lcons->Inherit(NanNew(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("LineString"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "addPoint", addPoint);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getPoint", getPoint);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getNumPoints", getNumPoints);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getLength", getLength);
	NODE_SET_PROTOTYPE_METHOD(lcons, "value", value);

	ATTR(lcons, "points", pointsGetter, READ_ONLY_SETTER);

	target->Set(NanNew("LineString"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

LineString::LineString(OGRLineString *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created LineString [%p]", geom);
}

LineString::LineString()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}

LineString::~LineString()
{
	if(this_) {
		LOG("Disposing LineString [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			NanAdjustExternalMemory(-size_);
		}
		LOG("Disposed LineString [%p]", this_);
		this_ = NULL;
	}
}

/**
 * Concrete representation of a multi-vertex line.
 *
 * @example
 * ```
 * var lineString = new gdal.LineString();
 * lineString.points.add(new gdal.Point(0,0));
 * lineString.points.add(new gdal.Point(0,10));```
 *
 * @constructor
 * @class gdal.LineString
 * @extends gdal.Geometry
 */
NAN_METHOD(LineString::New)
{
	NanScope();
	LineString *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<LineString *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("LineString constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new LineString(new OGRLineString());
	}

	Handle<Value> points = LineStringPoints::New(args.This());
	args.This()->SetHiddenValue(NanNew("points_"), points);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> LineString::New(OGRLineString *geom)
{
	NanEscapableScope();
	return NanEscapeScope(LineString::New(geom, true));
}

Handle<Value> LineString::New(OGRLineString *geom, bool owned)
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
		geom = static_cast<OGRLineString*>(geom->clone());
	};

	LineString *wrapped = new LineString(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(LineString::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(LineString::toString)
{
	NanScope();
	NanReturnValue(NanNew("LineString"));
}

/**
 * Computes the length of the line string.
 *
 * @method getLength
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(LineString, getLength, Number, get_Length);

/**
 * Returns the number of the points that make up the line string.
 *
 * @method getNumPoints
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(LineString, getNumPoints, Integer, getNumPoints);

/**
 * Returns the point at an index.
 *
 * @method getPoint
 * @param {integer} index 0-based index
 * @return {gdal.Point}
 */
NAN_METHOD(LineString::getPoint)
{
	NanScope();

	LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

	OGRPoint *pt = new OGRPoint();
	int i;

	NODE_ARG_INT(0, "i", i);

	geom->this_->getPoint(i, pt);

	NanReturnValue(Point::New(pt));
}

/**
 * Returns the point at the specified distance along the line string.
 *
 * @method value
 * @param {Number} distance
 * @return {gdal.Point}
 */
NAN_METHOD(LineString::value)
{
	NanScope();

	LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

	OGRPoint *pt = new OGRPoint();
	double dist;

	NODE_ARG_DOUBLE(0, "distance", dist);

	geom->this_->Value(dist, pt);

	NanReturnValue(Point::New(pt));
}

NAN_METHOD(LineString::addPoint)
{
	NanScope();

	LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());

	if (args[0]->IsNumber()) {

		double x = 0, y = 0, z = 0;
		NODE_ARG_DOUBLE(0, "x", x);
		NODE_ARG_DOUBLE(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);
		if (args.Length() < 3) {
			geom->this_->addPoint(x, y);
		} else {
			geom->this_->addPoint(x, y, z);
		}

	} else {

		Point* pt;
		NODE_ARG_WRAPPED(0, "point", Point, pt);
		geom->this_->addPoint(pt->get());

	}

	NanReturnUndefined();
}

/**
 * Points that make up the line string.
 *
 * @attribute points
 * @type {gdal.PointCollection}
 */
NAN_GETTER(LineString::pointsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("points_")));
}

} // namespace node_gdal