
#include "gdal_common.hpp"
#include "gdal_feature.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_geometry.hpp"
#include "gdal_field_defn.hpp"
#include "gdal_layer.hpp"
#include "collections/feature_fields.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Feature::constructor;

void Feature::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Feature::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Feature").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "getGeometry", getGeometry);
	//Nan::SetPrototypeMethod(lcons, "setGeometryDirectly", setGeometryDirectly);
	Nan::SetPrototypeMethod(lcons, "setGeometry", setGeometry);
	// Nan::SetPrototypeMethod(lcons, "stealGeometry", stealGeometry);
	Nan::SetPrototypeMethod(lcons, "clone", clone);
	//Nan::SetPrototypeMethod(lcons, "equals", equals);
	//Nan::SetPrototypeMethod(lcons, "getFieldDefn", getFieldDefn); (use defn.fields.get() instead)
	Nan::SetPrototypeMethod(lcons, "setFrom", setFrom);

	//Note: We should let node GC handle destroying features when they arent being used
	//TODO: Give node more info on the amount of memory a feature is using
	//      Nan::AdjustExternalMemory()
	//Nan::SetPrototypeMethod(lcons, "destroy", destroy);

	ATTR(lcons, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "defn", defnGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fid", fidGetter, fidSetter);

	target->Set(Nan::New("Feature").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

Feature::Feature(OGRFeature *feature)
	: Nan::ObjectWrap(),
	  this_(feature),
	  owned_(true)
{
	LOG("Created Feature[%p]", feature);
}

Feature::Feature()
	: Nan::ObjectWrap(),
	  this_(0),
	  owned_(true)
{
}

Feature::~Feature()
{
	dispose();
}

void Feature::dispose()
{
	if(this_) {
		LOG("Disposing Feature [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if(owned_) OGRFeature::DestroyFeature(this_);
		LOG("Disposed Feature [%p]", this_);
		this_ = NULL;
	}
}

/**
 * A simple feature, including geometry and attributes. Its fields and geometry type is defined by the given definition.
 *
 * ```
 * //create layer and specify geometry type
 * var layer = dataset.layers.create('mylayer', null, gdal.Point);
 *
 * //setup fields for the given layer
 * layer.fields.add(new gdal.FieldDefn('elevation', gdal.OFTInteger));
 * layer.fields.add(new gdal.FieldDefn('name', gdal.OFTString));
 *
 * //create feature using layer definition and then add it to the layer
 * var feature = new gdal.Feature(layer);
 * feature.fields.set('elevation', 13775);
 * feature.fields.set('name', 'Grand Teton');
 * feature.setGeometry(new gdal.Point(43.741208, -110.802414));
 * layer.features.add(feature);```
 *
 * @constructor
 * @class gdal.Feature
 * @param {gdal.Layer|gdal.FeatureDefn} definition
 */
NAN_METHOD(Feature::New)
{
	Nan::HandleScope scope;
	Feature* f;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Feature *>(ptr);

	} else {

		if (info.Length() < 1) {
			Nan::ThrowError("Constructor expects Layer or FeatureDefn object");
			return;
		}

		OGRFeatureDefn *def;

		if (IS_WRAPPED(info[0], Layer)) {
			Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(info[0].As<Object>());
			if (!layer->isAlive()) {
				Nan::ThrowError("Layer object already destroyed");
				return;
			}
			def = layer->get()->GetLayerDefn();
		} else if(IS_WRAPPED(info[0], FeatureDefn)) {
			FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(info[0].As<Object>());
			if (!feature_def->isAlive()) {
				Nan::ThrowError("FeatureDefn object already destroyed");
				return;
			}
			def = feature_def->get();
		} else {
			Nan::ThrowError("Constructor expects Layer or FeatureDefn object");
			return;
		}

		OGRFeature *ogr_f = new OGRFeature(def);
		f = new Feature(ogr_f);
	}

	Local<Value> fields = FeatureFields::New(info.This());
	Nan::SetPrivate(info.This(), Nan::New("fields_").ToLocalChecked(), fields);

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> Feature::New(OGRFeature *feature)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(Feature::New(feature, true));
}

Local<Value> Feature::New(OGRFeature *feature, bool owned)
{
	Nan::EscapableHandleScope scope;

	if (!feature) {
		return scope.Escape(Nan::Null());
	}

	Feature *wrapped = new Feature(feature);
	wrapped->owned_ = owned;
	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Feature::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	return scope.Escape(obj);
}

NAN_METHOD(Feature::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("Feature").ToLocalChecked());
}

/**
 * Returns the geometry of the feature.
 *
 * @method getGeometry
 * @return {gdal.Geometry}
 */
NAN_METHOD(Feature::getGeometry)
{
	Nan::HandleScope scope;

	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	OGRGeometry* geom = feature->this_->GetGeometryRef();
	if (!geom) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}


	info.GetReturnValue().Set(Geometry::New(geom, false));
}

/**
 * Returns the definition of a particular field at an index.
 *
 * @method getFieldDefn
 * @param {Integer} index Field index (0-based)
 * @return {gdal.FieldDefn}
 */
NAN_METHOD(Feature::getFieldDefn)
{
	Nan::HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		Nan::ThrowError("Invalid field index");
		return;
	}

	info.GetReturnValue().Set(FieldDefn::New(feature->this_->GetFieldDefnRef(field_index), false));
}

//NODE_WRAPPED_METHOD_WITH_RESULT(Feature, stealGeometry, Geometry, StealGeometry);

/**
 * Sets the feature's geometry.
 *
 * @throws Error
 * @method setGeometry
 * @param {gdal.Geometry} geometry
 */
NAN_METHOD(Feature::setGeometry)
{
	Nan::HandleScope scope;

	Geometry *geom = NULL;
	NODE_ARG_WRAPPED_OPT(0, "geometry", Geometry, geom);

	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	OGRErr err = feature->this_->SetGeometry(geom ? geom->get() : NULL);
	if(err){
		NODE_THROW_OGRERR(err);
	} 

	return;
}


/**
 * Determines if the features are the same.
 *
 * @method equals
 * @param {gdal.Feature} feature
 * @return {Boolean} `true` if the features are the same, `false` if different
 */
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Feature, equals, Boolean, Equal, Feature, "feature");

/**
 * Clones the feature.
 *
 * @method clone
 * @return {gdal.Feature}
 */
NAN_METHOD(Feature::clone)
{
	Nan::HandleScope scope;
	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}
	info.GetReturnValue().Set(Feature::New(feature->this_->Clone()));
}

/**
 * Releases the feature from memory.
 *
 * @method destroy
 */
NAN_METHOD(Feature::destroy)
{
	Nan::HandleScope scope;
	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}
	feature->dispose();
	return;
}

/**
 * Set one feature from another. Overwrites the contents of this feature
 * from the geometry and attributes of another.
 *
 * @example
 * ```
 * var feature1 = new gdal.Feature(defn);
 * var feature2 = new gdal.Feature(defn);
 * feature1.setGeometry(new gdal.Point(5, 10));
 * feature1.fields.set([5, 'test', 3.14]);
 * feature2.setFrom(feature1);```
 *
 * @throws Error
 * @method setFrom
 * @param {gdal.Feature} feature
 * @param {Array} [*index_map] Array of the indices (integers) of the feature's fields stored at the corresponding index of the source feature's fields. A value of -1 should be used to ignore the source's field. The array should not be `null` and be as long as the number of fields in the source feature.
 * @param {Boolean} [forgiving=true] `true` if the operation should continue despite lacking output fields matching some of the source fields.
 */
NAN_METHOD(Feature::setFrom)
{
	Nan::HandleScope scope;
	Feature *other_feature;
	int forgiving = 1;
	Local<Array> index_map;
	OGRErr err = 0;

	NODE_ARG_WRAPPED(0, "feature", Feature, other_feature);

	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	if (info.Length() <= 2) {
		NODE_ARG_BOOL_OPT(1, "forgiving", forgiving);

		err = feature->this_->SetFrom(other_feature->this_, forgiving ? TRUE : FALSE);
	} else {
		NODE_ARG_ARRAY(1, "index map", index_map);
		NODE_ARG_BOOL_OPT(2, "forgiving", forgiving);

		if (index_map->Length() < 1) {
			Nan::ThrowError("index map must contain at least 1 index");
			return;
		}

		int *index_map_ptr = new int[index_map->Length()];

		for (unsigned index = 0; index < index_map->Length(); index++) {
			Local<Value> field_index(index_map->Get(Nan::New<Integer>(index)));

			if (!field_index->IsUint32()) {
				delete [] index_map_ptr;
				Nan::ThrowError("index map must contain only integer values");
				return;
			}

			int val = (int)field_index->Uint32Value(); //todo: validate index? perhaps ogr already does this and throws an error

			index_map_ptr[index] = val;
		}

		err = feature->this_->SetFrom(other_feature->this_, index_map_ptr, forgiving ? TRUE : FALSE);

		delete [] index_map_ptr;
	}

	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	return;
}

/**
 * @readOnly
 * @attribute fields
 * @type {gdal.FeatureFields}
 */
NAN_GETTER(Feature::fieldsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("fields_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @attribute fid
 * @type {Number}
 */
NAN_GETTER(Feature::fidGetter)
{
	Nan::HandleScope scope;
	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}
	info.GetReturnValue().Set(Nan::New<Number>(feature->this_->GetFID()));
}

/**
 * @readOnly
 * @attribute defn
 * @type {gdal.FeatureDefn}
 */
NAN_GETTER(Feature::defnGetter)
{
	Nan::HandleScope scope;
	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}
	info.GetReturnValue().Set(FeatureDefn::New(feature->this_->GetDefnRef(), false));
}

NAN_SETTER(Feature::fidSetter)
{
	Nan::HandleScope scope;
	Feature *feature = Nan::ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}
	if(!value->IsInt32()){
		Nan::ThrowError("fid must be an integer");
		return;
	}
	feature->this_->SetFID(value->IntegerValue());
}

} // namespace node_gdal
