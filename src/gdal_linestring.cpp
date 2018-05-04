
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_linestring.hpp"
#include "gdal_point.hpp"
#include "collections/linestring_points.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> LineString::constructor;

void LineString::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(LineString::New);
	lcons->Inherit(Nan::New(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("LineString").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getLength", getLength);
	Nan::SetPrototypeMethod(lcons, "value", value);
	Nan::SetPrototypeMethod(lcons, "addSubLineString", addSubLineString);

	ATTR(lcons, "points", pointsGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("LineString").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

LineString::LineString(OGRLineString *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created LineString [%p]", geom);
}

LineString::LineString()
	: Nan::ObjectWrap(),
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
			Nan::AdjustExternalMemory(-size_);
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
	Nan::HandleScope scope;
	LineString *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<LineString *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("LineString constructor doesn't take any arguments");
			return;
		}
		f = new LineString(new OGRLineString());
	}

	Local<Value> points = LineStringPoints::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("points_").ToLocalChecked(), points);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> LineString::New(OGRLineString *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(LineString::New(geom, true));
}

Local<Value> LineString::New(OGRLineString *geom, bool owned)
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
		geom = static_cast<OGRLineString*>(geom->clone());
	};

	LineString *wrapped = new LineString(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(LineString::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(LineString::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("LineString").ToLocalChecked());
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
	Nan::HandleScope scope;

	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(info.This());

	OGRPoint *pt = new OGRPoint();
	double dist;

	NODE_ARG_DOUBLE(0, "distance", dist);

	geom->this_->Value(dist, pt);

	info.GetReturnValue().Set(Point::New(pt));
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
	Nan::HandleScope scope;

	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(info.This());
	LineString *other;
	int start = 0;
	int end = -1;
	
	NODE_ARG_WRAPPED(0, "line", LineString, other);
	NODE_ARG_INT_OPT(1, "start", start);
	NODE_ARG_INT_OPT(2, "end", end);

	int n = other->get()->getNumPoints();

	if(start < 0 || end < -1 || start >= n || end >= n) {
		Nan::ThrowRangeError("Invalid start or end index for linestring");
		return;
	}

	geom->this_->addSubLineString(other->get(), start, end);

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(geom);

	return;
}

/**
 * Points that make up the line string.
 *
 * @attribute points
 * @type {gdal.LineStringPoints}
 */
NAN_GETTER(LineString::pointsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("points_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
