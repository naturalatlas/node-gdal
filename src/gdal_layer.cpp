
#include "gdal_common.hpp"
#include "gdal_layer.hpp"
#include "gdal_feature.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_field_defn.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_dataset.hpp"
#include "gdal_geometry.hpp"
#include "collections/layer_features.hpp"
#include "collections/layer_fields.hpp"

#include <stdlib.h>
#include <sstream>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Layer::constructor;
ObjectCache<OGRLayer, Layer> Layer::cache;

void Layer::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Layer::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Layer").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getExtent", getExtent);
	Nan::SetPrototypeMethod(lcons, "setAttributeFilter", setAttributeFilter);
	Nan::SetPrototypeMethod(lcons, "setSpatialFilter", setSpatialFilter);
	Nan::SetPrototypeMethod(lcons, "getSpatialFilter", getSpatialFilter);
	Nan::SetPrototypeMethod(lcons, "testCapability", testCapability);
	Nan::SetPrototypeMethod(lcons, "flush", syncToDisk);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR_DONT_ENUM(lcons, "_uid", uidGetter, READ_ONLY_SETTER);
	ATTR(lcons, "srs", srsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "features", featuresGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "name", nameGetter, READ_ONLY_SETTER);
	ATTR(lcons, "geomType", geomTypeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "geomColumn", geomColumnGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fidColumn", fidColumnGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("Layer").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

Layer::Layer(OGRLayer *layer)
	: Nan::ObjectWrap(),
	  uid(0),
	  this_(layer),
	  parent_ds(0)
{
	LOG("Created layer [%p]", layer);
}

Layer::Layer()
	: Nan::ObjectWrap(),
	  uid(0),
	  this_(0),
	  parent_ds(0)
{
}

Layer::~Layer()
{
	dispose();
}

void Layer::dispose()
{
	if (this_) {

		LOG("Disposing layer [%p]", this_);

		ptr_manager.dispose(uid);

		LOG("Disposed layer [%p]", this_);
		this_ = NULL;
	}
};

/**
 * A representation of a layer of simple vector features, with access methods.
 *
 * @class gdal.Layer
 */
NAN_METHOD(Layer::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		Layer *f = static_cast<Layer *>(ptr);
		f->Wrap(info.This());

		Local<Value> features = LayerFeatures::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("features_").ToLocalChecked(), features);

		Local<Value> fields = LayerFields::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("fields_").ToLocalChecked(), fields);

		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create layer directly. Create with dataset instead.");
		return;
	}

	info.GetReturnValue().Set(info.This());
}

#if GDAL_VERSION_MAJOR >= 2
Local<Value> Layer::New(OGRLayer *raw, GDALDataset *raw_parent)
#else
Local<Value> Layer::New(OGRLayer *raw, OGRDataSource *raw_parent)
#endif
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(Layer::New(raw, raw_parent, false));
}

#if GDAL_VERSION_MAJOR >= 2
Local<Value> Layer::New(OGRLayer *raw, GDALDataset *raw_parent, bool result_set)
#else
Local<Value> Layer::New(OGRLayer *raw, OGRDataSource *raw_parent, bool result_set)
#endif
{
	Nan::EscapableHandleScope scope;

	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	if (cache.has(raw)) {
		return scope.Escape(cache.get(raw));
	}

	Layer *wrapped = new Layer(raw);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Layer::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	cache.add(raw, obj);

	//add reference to datasource so datasource doesnt get GC'ed while layer is alive
	Local<Object> ds;
	#if GDAL_VERSION_MAJOR >= 2
		if (Dataset::dataset_cache.has(raw_parent)) {
			ds = Dataset::dataset_cache.get(raw_parent);
		}
	#else
		if (Dataset::datasource_cache.has(raw_parent)) {
			ds = Dataset::datasource_cache.get(raw_parent);
		}
	#endif
		else {
			LOG("Layer's parent dataset disappeared from cache (layer = %p, dataset = %p)", raw, raw_parent);
			Nan::ThrowError("Layer's parent dataset disappeared from cache");
			return scope.Escape(Nan::Undefined());
			//ds = Dataset::New(raw_parent); //should never happen
		}

	long parent_uid = Nan::ObjectWrap::Unwrap<Dataset>(ds)->uid;
	
	wrapped->uid = ptr_manager.add(raw, parent_uid, result_set);
	wrapped->parent_ds = raw_parent;
	Nan::SetPrivate(obj, Nan::New("ds_").ToLocalChecked(), ds);

	return scope.Escape(obj);
}

NAN_METHOD(Layer::toString)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->this_) {
		info.GetReturnValue().Set(Nan::New("Null layer").ToLocalChecked());
		return;
	}

	std::ostringstream ss;
	ss << "Layer (" << layer->this_->GetName() << ")";

	info.GetReturnValue().Set(SafeString::New(ss.str().c_str()));
}

/**
 * Flush pending changes to disk.
 *
 * @throws Error
 * @method flush
 */
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(Layer, syncToDisk, SyncToDisk);

/**
 * Determines if the dataset supports the indicated operation.
 *
 * @method testCapability
 * @param {string} capability (see {{#crossLink "Constants (OLC)"}}capability list{{/crossLink}})
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(Layer, testCapability, Boolean, TestCapability, "capability");

/**
 * Fetch the extent of this layer.
 *
 * @throws Error
 * @method getExtent
 * @param {Boolean} [force=true]
 * @return {gdal.Envelope} Bounding envelope
 */
NAN_METHOD(Layer::getExtent)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}

	int force = 1;
	NODE_ARG_BOOL_OPT(0, "force", force);

	OGREnvelope *envelope = new OGREnvelope();
	OGRErr err = layer->this_->GetExtent(envelope, force);
	if(err) {
		Nan::ThrowError("Can't get layer extent without computing it");
		return;
	}

	Local<Object> obj = Nan::New<Object>();
	obj->Set(Nan::New("minX").ToLocalChecked(), Nan::New<Number>(envelope->MinX));
	obj->Set(Nan::New("maxX").ToLocalChecked(), Nan::New<Number>(envelope->MaxX));
	obj->Set(Nan::New("minY").ToLocalChecked(), Nan::New<Number>(envelope->MinY));
	obj->Set(Nan::New("maxY").ToLocalChecked(), Nan::New<Number>(envelope->MaxY));

	delete envelope;

	info.GetReturnValue().Set(obj);
}

/**
 * This method returns the current spatial filter for this layer.
 *
 * @throws Error
 * @method getSpatialFilter
 * @return {gdal.Geometry}
 */
NAN_METHOD(Layer::getSpatialFilter)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(Geometry::New(layer->this_->GetSpatialFilter(), false));
}

/**
 * This method sets the geometry to be used as a spatial filter when fetching
 * features via the `layer.features.next()` method. Only features that geometrically
 * intersect the filter geometry will be returned.
 *
 * Alernatively you can pass it envelope bounds as individual arguments.
 *
 * @example
 * ```
 * layer.setSpatialFilter(geometry);
 * layer.setSpatialFilter(minX, minY, maxX, maxY);```
 *
 * @throws Error
 * @method setSpatialFilter
 * @param {gdal.Geometry} filter
 */
NAN_METHOD(Layer::setSpatialFilter)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}

	if(info.Length() == 1) {
		Geometry *filter = NULL;
		NODE_ARG_WRAPPED_OPT(0, "filter", Geometry, filter);

		if(filter) {
			layer->this_->SetSpatialFilter(filter->get());
		} else {
			layer->this_->SetSpatialFilter(NULL);
		}
	} else if(info.Length() == 4) {
		double minX, minY, maxX, maxY;
		NODE_ARG_DOUBLE(0, "minX", minX);
		NODE_ARG_DOUBLE(1, "minY", minY);
		NODE_ARG_DOUBLE(2, "maxX", maxX);
		NODE_ARG_DOUBLE(3, "maxY", maxY);

		layer->this_->SetSpatialFilterRect(minX, minY, maxX, maxY);
	} else {
		Nan::ThrowError("Invalid number of arguments");
		return;
	}

	return;
}

/**
 * Sets the attribute query string to be used when fetching features via the
 * `layer.features.next()` method. Only features for which the query evaluates as
 * `true` will be returned.
 *
 * The query string should be in the format of an SQL WHERE clause. For instance
 * "population > 1000000 and population < 5000000" where `population` is an
 * attribute in the layer. The query format is normally a restricted form of
 * SQL WHERE clause as described in the "WHERE" section of the [OGR SQL
 * tutorial](http://www.gdal.org/ogr_sql.html). In some cases (RDBMS backed
 * drivers) the native capabilities of the database may be used to interprete
 * the WHERE clause in which case the capabilities will be broader than those
 * of OGR SQL.
 *
 * @example
 * ```
 * layer.setAttributeFilter('population > 1000000 and population < 5000000');```
 *
 * @throws Error
 * @method setAttributeFilter
 * @param {String} filter
 */
NAN_METHOD(Layer::setAttributeFilter)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}

	std::string filter = "";
	NODE_ARG_OPT_STR(0, "filter", filter);

	OGRErr err;
	if(filter.empty()){
		err = layer->this_->SetAttributeFilter(NULL);
	} else {
		err = layer->this_->SetAttributeFilter(filter.c_str());
	}

	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	return;
}

/*
NAN_METHOD(Layer::getLayerDefn)
{
	Nan::HandleScope scope;

	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());

	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	info.GetReturnValue().Set(FeatureDefn::New(layer->this_->GetLayerDefn(), false));
}*/

/**
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(Layer::dsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("ds_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute srs
 * @type {gdal.SpatialReference}
 */
NAN_GETTER(Layer::srsGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(SpatialReference::New(layer->this_->GetSpatialRef(), false));
}

/**
 * @readOnly
 * @attribute name
 * @type {String}
 */
NAN_GETTER(Layer::nameGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(SafeString::New(layer->this_->GetName()));
}

/**
 * @readOnly
 * @attribute geomColumn
 * @type {String}
 */
NAN_GETTER(Layer::geomColumnGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(SafeString::New(layer->this_->GetGeometryColumn()));
}

/**
 * @readOnly
 * @attribute fidColumn
 * @type {String}
 */
NAN_GETTER(Layer::fidColumnGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(SafeString::New(layer->this_->GetFIDColumn()));
}

/**
 * @readOnly
 * @attribute geomType
 * @type {Integer} (see {{#crossLink "Constants (wkb)"}}geometry types{{/crossLink}})
 */
NAN_GETTER(Layer::geomTypeGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(Nan::New<Integer>(layer->this_->GetGeomType()));
}

/**
 * @readOnly
 * @attribute features
 * @type {gdal.LayerFeatures}
 */
NAN_GETTER(Layer::featuresGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("features_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute fields
 * @type {gdal.LayerFields}
 */
NAN_GETTER(Layer::fieldsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("fields_").ToLocalChecked()).ToLocalChecked());
}

NAN_GETTER(Layer::uidGetter)
{
	Nan::HandleScope scope;
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info.This());
	info.GetReturnValue().Set(Nan::New((int)layer->uid));
}

} // namespace node_gdal
