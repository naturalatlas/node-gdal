
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipoint.hpp"
#include "gdal_point.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MultiPoint::constructor;

void MultiPoint::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(MultiPoint::New);
	lcons->Inherit(NanNew(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("MultiPoint"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);

	target->Set(NanNew("MultiPoint"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

MultiPoint::MultiPoint(OGRMultiPoint *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiPoint [%p]", geom);
}

MultiPoint::MultiPoint()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


MultiPoint::~MultiPoint()
{
	if(this_) {
		LOG("Disposing MultiPoint [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			NanAdjustExternalMemory(-size_);
		}
		LOG("Disposed MultiPoint [%p]", this_);
		this_ = NULL;
	}
}

/**
 * @constructor
 * @class gdal.MultiPoint
 * @extends gdal.GeometryCollection
 */
NAN_METHOD(MultiPoint::New)
{
	NanScope();
	MultiPoint *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiPoint *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("MultiPoint constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new MultiPoint(new OGRMultiPoint());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This());
	args.This()->SetHiddenValue(NanNew("children_"), children);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> MultiPoint::New(OGRMultiPoint *geom)
{
	NanEscapableScope();
	return NanEscapeScope(MultiPoint::New(geom, true));
}

Handle<Value> MultiPoint::New(OGRMultiPoint *geom, bool owned)
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
		geom = static_cast<OGRMultiPoint*>(geom->clone());
	}

	MultiPoint *wrapped = new MultiPoint(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(MultiPoint::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(MultiPoint::toString)
{
	NanScope();
	NanReturnValue(NanNew("MultiPoint"));
}

} // namespace node_gdal