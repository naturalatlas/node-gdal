
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> GeometryCollection::constructor;

void GeometryCollection::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(GeometryCollection::New);
	lcons->Inherit(Nan::New(Geometry::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("GeometryCollection").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getArea", getArea);
	Nan::SetPrototypeMethod(lcons, "getLength", getLength);

	ATTR(lcons, "children", childrenGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("GeometryCollection").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

GeometryCollection::GeometryCollection(OGRGeometryCollection *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created GeometryCollection [%p]", geom);
}

GeometryCollection::GeometryCollection()
	: Nan::ObjectWrap(),
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
			Nan::AdjustExternalMemory(-size_);
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
	Nan::HandleScope scope;
	GeometryCollection *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<GeometryCollection *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("GeometryCollection constructor doesn't take any arguments");
			return;
		}
		f = new GeometryCollection(new OGRGeometryCollection());
	}

	Local<Value> children = GeometryCollectionChildren::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("children_").ToLocalChecked(), children);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> GeometryCollection::New(OGRGeometryCollection *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(GeometryCollection::New(geom, true));
}

Local<Value> GeometryCollection::New(OGRGeometryCollection *geom, bool owned)
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
		geom = static_cast<OGRGeometryCollection*>(geom->clone());
	};

	GeometryCollection *wrapped = new GeometryCollection(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(GeometryCollection::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}


NAN_METHOD(GeometryCollection::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("GeometryCollection").ToLocalChecked());
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
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("children_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
