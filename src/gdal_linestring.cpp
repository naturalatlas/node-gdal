
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
	NODE_SET_PROTOTYPE_METHOD(lcons, "getLength", getLength);
	NODE_SET_PROTOTYPE_METHOD(lcons, "value", value);
	NODE_SET_PROTOTYPE_METHOD(lcons, "addSubLineString", addSubLineString);

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

/**
 * Add a segment of another linestring to this one.
 * 
 * Adds the request range of vertices to the end of this line string in an efficient manner. If the start index is larger than the end index then the vertices will be reversed as they are copied.
 *
 * @method addSubLineString
 * @param {gdal.LineString} line the other linestring
 * @param {int} [start=0] the first vertex to copy, defaults to 0 to start with the first vertex in the other linestring
 * @param {int} [end=-1] the last vertex to copy, defaults to -1 indicating the last vertex of the other linestring
 * @return {void}
 */
NAN_METHOD(LineString::addSubLineString)
{
	NanScope();

	LineString *geom = ObjectWrap::Unwrap<LineString>(args.This());
	LineString *other;
	int start = 0;
	int end = -1;
	
	NODE_ARG_WRAPPED(0, "line", LineString, other);
	NODE_ARG_INT_OPT(1, "start", start);
	NODE_ARG_INT_OPT(2, "end", end);

	int n = other->get()->getNumPoints();

	if(start < 0 || end < -1 || start >= n || end >= n) {
		NanThrowRangeError("Invalid start or end index for linestring");
		NanReturnUndefined();
	}

	geom->this_->addSubLineString(other->get(), start, end);

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(geom);

	NanReturnUndefined();
}

/**
 * Points that make up the line string.
 *
 * @attribute points
 * @type {gdal.LineStringPoints}
 */
NAN_GETTER(LineString::pointsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("points_")));
}

} // namespace node_gdal