
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_polygon.hpp"
#include "collections/polygon_rings.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> Polygon::constructor;

void Polygon::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(Polygon::New);
	lcons->Inherit(NanNew(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("Polygon"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getArea", getArea);

	ATTR(lcons, "rings", ringsGetter, READ_ONLY_SETTER);

	target->Set(NanNew("Polygon"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

Polygon::Polygon(OGRPolygon *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created Polygon [%p]", geom);
}

Polygon::Polygon()
	: ObjectWrap(),
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
			NanAdjustExternalMemory(-size_);
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
	NanScope();
	Polygon *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Polygon *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("Polygon constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new Polygon(new OGRPolygon());
	}

	Handle<Value> rings = PolygonRings::New(args.This());
	args.This()->SetHiddenValue(NanNew("rings_"), rings);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> Polygon::New(OGRPolygon *geom)
{
	NanEscapableScope();
	return NanEscapeScope(Polygon::New(geom, true));
}

Handle<Value> Polygon::New(OGRPolygon *geom, bool owned)
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
		geom = static_cast<OGRPolygon*>(geom->clone());
	}

	Polygon *wrapped = new Polygon(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Polygon::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(Polygon::toString)
{
	NanScope();
	NanReturnValue(NanNew("Polygon"));
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
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("rings_")));
}

} // namespace node_gdal