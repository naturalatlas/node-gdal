
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

Persistent<FunctionTemplate> Layer::constructor;
ObjectCache<OGRLayer, Layer> Layer::cache;

void Layer::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(Layer::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("Layer"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getExtent", getExtent);
	NODE_SET_PROTOTYPE_METHOD(lcons, "setAttributeFilter", setAttributeFilter);
	NODE_SET_PROTOTYPE_METHOD(lcons, "setSpatialFilter", setSpatialFilter);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getSpatialFilter", getSpatialFilter);
	NODE_SET_PROTOTYPE_METHOD(lcons, "testCapability", testCapability);
	NODE_SET_PROTOTYPE_METHOD(lcons, "flush", syncToDisk);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "srs", srsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "features", featuresGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "name", nameGetter, READ_ONLY_SETTER);
	ATTR(lcons, "geomType", geomTypeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "geomColumn", geomColumnGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fidColumn", fidColumnGetter, READ_ONLY_SETTER);

	target->Set(NanNew("Layer"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

Layer::Layer(OGRLayer *layer)
	: ObjectWrap(),
	  this_(layer),
	  parent_ds(0),
	  is_result_set(false)
{
	LOG("Created layer [%p]", layer);
}

Layer::Layer()
	: ObjectWrap(),
	  this_(0),
	  parent_ds(0),
	  is_result_set(false)
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

		cache.erase(this_);
		if (is_result_set && parent_ds && this_) {
			LOG("Releasing result set [%p] from datasource [%p]", this_, parent_ds);
			parent_ds->ReleaseResultSet(this_);
		}

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
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		Layer *f = static_cast<Layer *>(ptr);
		f->Wrap(args.This());

		Handle<Value> features = LayerFeatures::New(args.This());
		args.This()->SetHiddenValue(NanNew("features_"), features);

		Handle<Value> fields = LayerFields::New(args.This());
		args.This()->SetHiddenValue(NanNew("fields_"), fields);

		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create layer directly. Create with dataset instead.");
		NanReturnUndefined();
	}

	NanReturnValue(args.This());
}

#if GDAL_VERSION_MAJOR >= 2
Handle<Value> Layer::New(OGRLayer *raw, GDALDataset *raw_parent)
#else
Handle<Value> Layer::New(OGRLayer *raw, OGRDataSource *raw_parent)
#endif
{
	NanEscapableScope();
	return NanEscapeScope(Layer::New(raw, raw_parent, false));
}

#if GDAL_VERSION_MAJOR >= 2
Handle<Value> Layer::New(OGRLayer *raw, GDALDataset *raw_parent, bool result_set)
#else
Handle<Value> Layer::New(OGRLayer *raw, OGRDataSource *raw_parent, bool result_set)
#endif
{
	NanEscapableScope();

	if (!raw) {
		return NanEscapeScope(NanNull());
	}
	if (cache.has(raw)) {
		return NanEscapeScope(NanNew(cache.get(raw)));
	}

	Layer *wrapped = new Layer(raw);
	wrapped->is_result_set = result_set;

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Layer::constructor)->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	//add reference to datasource so datasource doesnt get GC'ed while layer is alive
	if (raw_parent) {
		Handle<Value> ds;
		#if GDAL_VERSION_MAJOR >= 2
			if (Dataset::dataset_cache.has(raw_parent)) {
				ds = NanNew(Dataset::dataset_cache.get(raw_parent));
			}
		#else
			if (Dataset::datasource_cache.has(raw_parent)) {
				ds = NanNew(Dataset::datasource_cache.get(raw_parent));
			}
		#endif
		else {
			LOG("Layer's parent dataset disappeared from cache (layer = %p, dataset = %p)", raw, raw_parent);
			NanThrowError("Layer's parent dataset disappeared from cache");
			return NanEscapeScope(NanUndefined());
			//ds = Dataset::New(raw_parent); //should never happen
		}

		wrapped->parent_ds = raw_parent;
		obj->SetHiddenValue(NanNew("ds_"), ds);
	}

	return NanEscapeScope(obj);
}

NAN_METHOD(Layer::toString)
{
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanReturnValue(NanNew("Null layer"));
	}

	std::ostringstream ss;
	ss << "Layer (" << layer->this_->GetName() << ")";

	NanReturnValue(SafeString::New(ss.str().c_str()));
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
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}

	int force = 1;
	NODE_ARG_BOOL_OPT(0, "force", force);

	OGREnvelope *envelope = new OGREnvelope();
	OGRErr err = layer->this_->GetExtent(envelope, force);
	if(err) {
		NanThrowError("Can't get layer extent without computing it");
		NanReturnUndefined();
	}

	Local<Object> obj = NanNew<Object>();
	obj->Set(NanNew("minX"), NanNew<Number>(envelope->MinX));
	obj->Set(NanNew("maxX"), NanNew<Number>(envelope->MaxX));
	obj->Set(NanNew("minY"), NanNew<Number>(envelope->MinY));
	obj->Set(NanNew("maxY"), NanNew<Number>(envelope->MaxY));

	delete envelope;

	NanReturnValue(obj);
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
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(Geometry::New(layer->this_->GetSpatialFilter(), false));
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
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}

	if(args.Length() == 1) {
		Geometry *filter = NULL;
		NODE_ARG_WRAPPED_OPT(0, "filter", Geometry, filter);

		if(filter) {
			layer->this_->SetSpatialFilter(filter->get());
		} else {
			layer->this_->SetSpatialFilter(NULL);
		}
	} else if(args.Length() == 4) {
		double minX, minY, maxX, maxY;
		NODE_ARG_DOUBLE(0, "minX", minX);
		NODE_ARG_DOUBLE(1, "minY", minY);
		NODE_ARG_DOUBLE(2, "maxX", maxX);
		NODE_ARG_DOUBLE(3, "maxY", maxY);

		layer->this_->SetSpatialFilterRect(minX, minY, maxX, maxY);
	} else {
		NanThrowError("Invalid number of arguments");
		NanReturnUndefined();
	}

	NanReturnUndefined();
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
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
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
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

/*
NAN_METHOD(Layer::getLayerDefn)
{
	NanScope();

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());

	if (!layer->this_) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(FeatureDefn::New(layer->this_->GetLayerDefn(), false));
}*/

/**
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(Layer::dsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("ds_")));
}

/**
 * @readOnly
 * @attribute srs
 * @type {gdal.SpatialReference}
 */
NAN_GETTER(Layer::srsGetter)
{
	NanScope();
	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(SpatialReference::New(layer->this_->GetSpatialRef(), false));
}

/**
 * @readOnly
 * @attribute name
 * @type {String}
 */
NAN_GETTER(Layer::nameGetter)
{
	NanScope();
	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(SafeString::New(layer->this_->GetName()));
}

/**
 * @readOnly
 * @attribute geomColumn
 * @type {String}
 */
NAN_GETTER(Layer::geomColumnGetter)
{
	NanScope();
	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(SafeString::New(layer->this_->GetGeometryColumn()));
}

/**
 * @readOnly
 * @attribute fidColumn
 * @type {String}
 */
NAN_GETTER(Layer::fidColumnGetter)
{
	NanScope();
	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(SafeString::New(layer->this_->GetFIDColumn()));
}

/**
 * @readOnly
 * @attribute geomType
 * @type {Integer} (see {{#crossLink "Constants (wkb)"}}geometry types{{/crossLink}})
 */
NAN_GETTER(Layer::geomTypeGetter)
{
	NanScope();
	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		NanThrowError("Layer object has already been destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(NanNew<Integer>(layer->this_->GetGeomType()));
}

/**
 * @readOnly
 * @attribute features
 * @type {gdal.LayerFeatures}
 */
NAN_GETTER(Layer::featuresGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("features_")));
}

/**
 * @readOnly
 * @attribute fields
 * @type {gdal.LayerFields}
 */
NAN_GETTER(Layer::fieldsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("fields_")));
}

} // namespace node_gdal