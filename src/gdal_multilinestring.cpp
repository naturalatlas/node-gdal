
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multilinestring.hpp"
#include "gdal_linestring.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MultiLineString::constructor;

void MultiLineString::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(MultiLineString::New);
	lcons->Inherit(NanNew(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("MultiLineString"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "polygonize", polygonize);

	target->Set(NanNew("MultiLineString"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

MultiLineString::MultiLineString(OGRMultiLineString *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiLineString [%p]", geom);
}

MultiLineString::MultiLineString()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}

MultiLineString::~MultiLineString()
{
	if(this_) {
		LOG("Disposing GeometryCollection [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if (owned_) {
			OGRGeometryFactory::destroyGeometry(this_);
			NanAdjustExternalMemory(-size_);
		}
		LOG("Disposed GeometryCollection [%p]", this_);
		this_ = NULL;
	}
}

/**
 * @constructor
 * @class gdal.MultiLineString
 * @extends gdal.GeometryCollection
 */
NAN_METHOD(MultiLineString::New)
{
	NanScope();
	MultiLineString *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiLineString *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("MultiLineString constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new MultiLineString(new OGRMultiLineString());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This());
	args.This()->SetHiddenValue(NanNew("children_"), children);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> MultiLineString::New(OGRMultiLineString *geom)
{
	NanEscapableScope();
	return NanEscapeScope(MultiLineString::New(geom, true));
}

Handle<Value> MultiLineString::New(OGRMultiLineString *geom, bool owned)
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
		geom = static_cast<OGRMultiLineString*>(geom->clone());
	};

	MultiLineString *wrapped = new MultiLineString(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(MultiLineString::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(MultiLineString::toString)
{
	NanScope();
	NanReturnValue(NanNew("MultiLineString"));
}

/**
 * Converts it to a polygon.
 *
 * @method polygonize
 * @return {gdal.Polygon}
 */
NAN_METHOD(MultiLineString::polygonize)
{
	NanScope();

	MultiLineString *geom = ObjectWrap::Unwrap<MultiLineString>(args.This());

	NanReturnValue(Geometry::New(geom->this_->Polygonize()));
}

} // namespace node_gdal