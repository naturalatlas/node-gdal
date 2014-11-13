
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> GeometryCollection::constructor;

void GeometryCollection::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(GeometryCollection::New);
	lcons->Inherit(NanNew(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("GeometryCollection"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getArea", getArea);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getLength", getLength);

	ATTR(lcons, "children", childrenGetter, READ_ONLY_SETTER);

	target->Set(NanNew("GeometryCollection"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

GeometryCollection::GeometryCollection(OGRGeometryCollection *geom)
	: ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created GeometryCollection [%p]", geom);
}

GeometryCollection::GeometryCollection()
	: ObjectWrap(),
	  this_(NULL),
	  owned_(true),
	  size_(0)
{
}


GeometryCollection::~GeometryCollection()
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
 * A collection of 1 or more geometry objects.
 *
 * @constructor
 * @class gdal.GeometryCollection
 * @extends gdal.Geometry
 */
NAN_METHOD(GeometryCollection::New)
{
	NanScope();
	GeometryCollection *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<GeometryCollection *>(ptr);

	} else {
		if (args.Length() != 0) {
			NanThrowError("GeometryCollection constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new GeometryCollection(new OGRGeometryCollection());
	}

	Handle<Value> children = GeometryCollectionChildren::New(args.This());
	args.This()->SetHiddenValue(NanNew("children_"), children);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> GeometryCollection::New(OGRGeometryCollection *geom)
{
	NanEscapableScope();
	return NanEscapeScope(GeometryCollection::New(geom, true));
}

Handle<Value> GeometryCollection::New(OGRGeometryCollection *geom, bool owned)
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
		geom = static_cast<OGRGeometryCollection*>(geom->clone());
	};

	GeometryCollection *wrapped = new GeometryCollection(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(GeometryCollection::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}


NAN_METHOD(GeometryCollection::toString)
{
	NanScope();
	NanReturnValue(NanNew("GeometryCollection"));
}

/**
 * Computes the combined area of the geometries.
 *
 * @method getArea
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(GeometryCollection, getArea, Number, get_Area);

/**
 * Compute the length of a multicurve.
 *
 * @method getLength
 * @return Number
 */
NODE_WRAPPED_METHOD_WITH_RESULT(GeometryCollection, getLength, Number, get_Length);

/**
 * All geometries represented by this collection.
 *
 * @attribute children
 * @type {gdal.GeometryCollectionChildren}
 */
NAN_GETTER(GeometryCollection::childrenGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("children_")));
}

} // namespace node_gdal