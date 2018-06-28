
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multipoint.hpp"
#include "gdal_point.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> MultiPoint::constructor;

void MultiPoint::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(MultiPoint::New);
	lcons->Inherit(Nan::New(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("MultiPoint").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);

	target->Set(Nan::New("MultiPoint").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

MultiPoint::MultiPoint(OGRMultiPoint *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiPoint [%p]", geom);
}

MultiPoint::MultiPoint()
	: Nan::ObjectWrap(),
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
			Nan::AdjustExternalMemory(-size_);
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
	Nan::HandleScope scope;
	MultiPoint *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiPoint *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("MultiPoint constructor doesn't take any arguments");
			return;
		}
		f = new MultiPoint(new OGRMultiPoint());
	}

	Local<Value> children = GeometryCollectionChildren::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("children_").ToLocalChecked(), children);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> MultiPoint::New(OGRMultiPoint *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(MultiPoint::New(geom, true));
}

Local<Value> MultiPoint::New(OGRMultiPoint *geom, bool owned)
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
		geom = static_cast<OGRMultiPoint*>(geom->clone());
	}

	MultiPoint *wrapped = new MultiPoint(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(MultiPoint::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(MultiPoint::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("MultiPoint").ToLocalChecked());
}

} // namespace node_gdal