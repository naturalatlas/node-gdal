#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipolygon.hpp"
#include "gdal_polygon.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MultiPolygon::constructor;

void MultiPolygon::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(MultiPolygon::New);
	lcons->Inherit(NanNew(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("MultiPolygon"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "unionCascaded", unionCascaded);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getArea", getArea);

	target->Set(NanNew("MultiPolygon"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

MultiPolygon::MultiPolygon(OGRMultiPolygon *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiPolygon [%p]", geom);
}

MultiPolygon::MultiPolygon()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}

MultiPolygon::~MultiPolygon()
{
	if(this_) {
		LOG("Disposing MultiPolygon [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			NanAdjustExternalMemory(-size_);
		}
		LOG("Disposed MultiPolygon [%p]", this_);
		this_ = NULL;
	}
}

/**
 * @constructor
 * @class gdal.MultiPolygon
 * @extends gdal.GeometryCollection
 */
NAN_METHOD(MultiPolygon::New)
{
	NanScope();
	MultiPolygon *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiPolygon *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("MultiPolygon constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new MultiPolygon(new OGRMultiPolygon());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This());
	args.This()->SetHiddenValue(NanNew("children_"), children);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> MultiPolygon::New(OGRMultiPolygon *geom)
{
	NanEscapableScope();
	return NanEscapeScope(MultiPolygon::New(geom, true));
}

Handle<Value> MultiPolygon::New(OGRMultiPolygon *geom, bool owned)
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
		geom = static_cast<OGRMultiPolygon*>(geom->clone());
	};

	MultiPolygon *wrapped = new MultiPolygon(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(MultiPolygon::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(MultiPolygon::toString)
{
	NanScope();
	NanReturnValue(NanNew("MultiPolygon"));
}

/**
 * Unions all the geometries and returns the result.
 *
 * @method unionCascaded
 * @return {gdal.Geometry}
 */
NAN_METHOD(MultiPolygon::unionCascaded)
{
	NanScope();

	MultiPolygon *geom = ObjectWrap::Unwrap<MultiPolygon>(args.This());

	NanReturnValue(Geometry::New(geom->this_->UnionCascaded()));
}

/**
 * Computes the combined area of the collection.
 *
 * @method getArea
 * @return {Number}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(MultiPolygon, getArea, Number, get_Area);

} // namespace node_gdal