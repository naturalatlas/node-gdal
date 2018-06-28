
#include "gdal_common.hpp"
#include "gdal_geometry.hpp"
#include "gdal_geometrycollection.hpp"
#include "gdal_multilinestring.hpp"
#include "gdal_linestring.hpp"
#include "collections/geometry_collection_children.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> MultiLineString::constructor;

void MultiLineString::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(MultiLineString::New);
	lcons->Inherit(Nan::New(GeometryCollection::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("MultiLineString").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "polygonize", polygonize);

	target->Set(Nan::New("MultiLineString").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

MultiLineString::MultiLineString(OGRMultiLineString *geom)
	: Nan::ObjectWrap(),
	  this_(geom),
	  owned_(true),
	  size_(0)
{
	LOG("Created MultiLineString [%p]", geom);
}

MultiLineString::MultiLineString()
	: Nan::ObjectWrap(),
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
			Nan::AdjustExternalMemory(-size_);
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
	Nan::HandleScope scope;
	MultiLineString *f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<MultiLineString *>(ptr);

	} else {
		if (info.Length() != 0) {
			Nan::ThrowError("MultiLineString constructor doesn't take any arguments");
			return;
		}
		f = new MultiLineString(new OGRMultiLineString());
	}

	Local<Value> children = GeometryCollectionChildren::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("children_").ToLocalChecked(), children);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> MultiLineString::New(OGRMultiLineString *geom)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(MultiLineString::New(geom, true));
}

Local<Value> MultiLineString::New(OGRMultiLineString *geom, bool owned)
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
		geom = static_cast<OGRMultiLineString*>(geom->clone());
	};

	MultiLineString *wrapped = new MultiLineString(geom);
	wrapped->owned_ = true;

	UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(wrapped);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(MultiLineString::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(MultiLineString::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("MultiLineString").ToLocalChecked());
}

/**
 * Converts it to a polygon.
 *
 * @method polygonize
 * @return {gdal.Polygon}
 */
NAN_METHOD(MultiLineString::polygonize)
{
	Nan::HandleScope scope;

	MultiLineString *geom = Nan::ObjectWrap::Unwrap<MultiLineString>(info.This());

	info.GetReturnValue().Set(Geometry::New(geom->this_->Polygonize()));
}

} // namespace node_gdal