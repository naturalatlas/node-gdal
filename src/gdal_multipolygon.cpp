#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipolygon.hpp"
#include "gdal_polygon.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> MultiPolygon::constructor;

void MultiPolygon::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(MultiPolygon::New);
	lcons->Inherit(Nan::New(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("MultiPolygon").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "unionCascaded", unionCascaded);
	Nan::SetPrototypeMethod(lcons, "getArea", getArea);

	target->Set(Nan::New("MultiPolygon").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

MultiPolygon::MultiPolygon(OGRMultiPolygon *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiPolygon [%p]", geom);
}

MultiPolygon::MultiPolygon()
	: Nan::ObjectWrap(),
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
			Nan::AdjustExternalMemory(-size_);
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
	Nan::HandleScope scope;
	MultiPolygon *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiPolygon *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("MultiPolygon constructor doesn't take any arguments");
			return;
		}
		f = new MultiPolygon(new OGRMultiPolygon());
	}

	Local<Value> children = GeometryCollectionChildren::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("children_").ToLocalChecked(), children);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> MultiPolygon::New(OGRMultiPolygon *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(MultiPolygon::New(geom, true));
}

Local<Value> MultiPolygon::New(OGRMultiPolygon *geom, bool owned)
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
		geom = static_cast<OGRMultiPolygon*>(geom->clone());
	};

	MultiPolygon *wrapped = new MultiPolygon(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(MultiPolygon::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(MultiPolygon::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("MultiPolygon").ToLocalChecked());
}

/**
 * Unions all the geometries and returns the result.
 *
 * @method unionCascaded
 * @return {gdal.Geometry}
 */
NAN_METHOD(MultiPolygon::unionCascaded)
{
	Nan::HandleScope scope;

	MultiPolygon *geom = Nan::ObjectWrap::Unwrap<MultiPolygon>(info.This());

	info.GetReturnValue().Set(Geometry::New(geom->this_->UnionCascaded()));
}

/**
 * Computes the combined area of the collection.
 *
 * @method getArea
 * @return {Number}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(MultiPolygon, getArea, Number, get_Area);

} // namespace node_gdal