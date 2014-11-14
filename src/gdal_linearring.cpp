
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_linearring.hpp"
#include "gdal_linestring.hpp"
#include "collections/linestring_points.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> LinearRing::constructor;

void LinearRing::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(LinearRing::New);
	lcons->Inherit(NanNew(LineString::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("LinearRing"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getArea", getArea);

	target->Set(NanNew("LinearRing"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

LinearRing::LinearRing(OGRLinearRing *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created LinearRing [%p]", geom);
}

LinearRing::LinearRing()
	: ObjectWrap(),
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
			NanAdjustExternalMemory(-size_);
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
	NanScope();
	LinearRing *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<LinearRing *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("LinearRing constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new LinearRing(new OGRLinearRing());
	}

	Handle<Value> points = LineStringPoints::New(args.This());
	args.This()->SetHiddenValue(NanNew("points_"), points);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> LinearRing::New(OGRLinearRing *geom)
{
	NanEscapableScope();
	return NanEscapeScope(LinearRing::New(geom, true));
}

Handle<Value> LinearRing::New(OGRLinearRing *geom, bool owned)
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
		geom = static_cast<OGRLinearRing*>(geom->clone());
	};

	LinearRing *wrapped = new LinearRing(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(LinearRing::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(LinearRing::toString)
{
	NanScope();
	NanReturnValue(NanNew("LinearRing"));
}

/**
 * Computes the area enclosed by the ring.
 *
 * @method getArea
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(LinearRing, getArea, Number, get_Area);

} // namespace node_gdal