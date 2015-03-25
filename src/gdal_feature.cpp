
#include "gdal_common.hpp"
#include "gdal_feature.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_geometry.hpp"
#include "gdal_field_defn.hpp"
#include "gdal_layer.hpp"
#include "collections/feature_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> Feature::constructor;

void Feature::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(Feature::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("Feature"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getGeometry", getGeometry);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "setGeometryDirectly", setGeometryDirectly);
	NODE_SET_PROTOTYPE_METHOD(lcons, "setGeometry", setGeometry);
	// NODE_SET_PROTOTYPE_METHOD(lcons, "stealGeometry", stealGeometry);
	NODE_SET_PROTOTYPE_METHOD(lcons, "clone", clone);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "equals", equals);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "getFieldDefn", getFieldDefn); (use defn.fields.get() instead)
	NODE_SET_PROTOTYPE_METHOD(lcons, "setFrom", setFrom);

	//Note: We should let node GC handle destroying features when they arent being used
	//TODO: Give node more info on the amount of memory a feature is using
	//      NanAdjustExternalMemory()
	//NODE_SET_PROTOTYPE_METHOD(lcons, "destroy", destroy);

	ATTR(lcons, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "defn", defnGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fid", fidGetter, fidSetter);

	target->Set(NanNew("Feature"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

Feature::Feature(OGRFeature *feature)
	: ObjectWrap(),
	  this_(feature),
	  owned_(true)
{
	LOG("Created Feature[%p]", feature);
}

Feature::Feature()
	: ObjectWrap(),
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
	NanScope();
	Feature* f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<Feature *>(ptr);

	} else {

		if (args.Length() < 1) {
			NanThrowError("Constructor expects Layer or FeatureDefn object");
			NanReturnUndefined();
		}

		OGRFeatureDefn *def;

		if (IS_WRAPPED(args[0], Layer)) {
			Layer *layer = ObjectWrap::Unwrap<Layer>(args[0].As<Object>());
			if (!layer->get()) {
				NanThrowError("Layer object already destroyed");
				NanReturnUndefined();
			}
			def = layer->get()->GetLayerDefn();
		} else if(IS_WRAPPED(args[0], FeatureDefn)) {
			FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(args[0].As<Object>());
			if (!feature_def->get()) {
				NanThrowError("FeatureDefn object already destroyed");
				NanReturnUndefined();
			}
			def = feature_def->get();
		} else {
			NanThrowError("Constructor expects Layer or FeatureDefn object");
			NanReturnUndefined();
		}

		OGRFeature *ogr_f = new OGRFeature(def);
		f = new Feature(ogr_f);
	}

	Handle<Value> fields = FeatureFields::New(args.This());
	args.This()->SetHiddenValue(NanNew("fields_"), fields);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> Feature::New(OGRFeature *feature)
{
	NanEscapableScope();
	return NanEscapeScope(Feature::New(feature, true));
}

Handle<Value> Feature::New(OGRFeature *feature, bool owned)
{
	NanEscapableScope();

	if (!feature) {
		return NanEscapeScope(NanNull());
	}

	Feature *wrapped = new Feature(feature);
	wrapped->owned_ = owned;
	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Feature::constructor)->GetFunction()->NewInstance(1, &ext);
	return NanEscapeScope(obj);
}

NAN_METHOD(Feature::toString)
{
	NanScope();
	NanReturnValue(NanNew("Feature"));
}

/**
 * Returns the geometry of the feature.
 *
 * @method getGeometry
 * @return {gdal.Geometry}
 */
NAN_METHOD(Feature::getGeometry)
{
	NanScope();

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	OGRGeometry* geom = feature->this_->GetGeometryRef();
	if (!geom) {
		NanReturnNull();
	}


	NanReturnValue(Geometry::New(geom, false));
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
	NanScope();
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		NanThrowError("Invalid field index");
		NanReturnUndefined();
	}

	NanReturnValue(FieldDefn::New(feature->this_->GetFieldDefnRef(field_index), false));
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
	NanScope();

	Geometry *geom = NULL;
	NODE_ARG_WRAPPED_OPT(0, "geometry", Geometry, geom);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	OGRErr err = feature->this_->SetGeometry(geom ? geom->get() : NULL);
	if(err){
		NODE_THROW_OGRERR(err);
	} 

	NanReturnUndefined();
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
	NanScope();
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(Feature::New(feature->this_->Clone()));
}

/**
 * Releases the feature from memory.
 *
 * @method destroy
 */
NAN_METHOD(Feature::destroy)
{
	NanScope();
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}
	feature->dispose();
	NanReturnUndefined();
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
	NanScope();
	Feature *other_feature;
	int forgiving = 1;
	Handle<Array> index_map;
	OGRErr err = 0;

	NODE_ARG_WRAPPED(0, "feature", Feature, other_feature);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() <= 2) {
		NODE_ARG_BOOL_OPT(1, "forgiving", forgiving);

		err = feature->this_->SetFrom(other_feature->this_, forgiving ? TRUE : FALSE);
	} else {
		NODE_ARG_ARRAY(1, "index map", index_map);
		NODE_ARG_BOOL_OPT(2, "forgiving", forgiving);

		if (index_map->Length() < 1) {
			NanThrowError("index map must contain at least 1 index");
			NanReturnUndefined();
		}

		int *index_map_ptr = new int[index_map->Length()];

		for (unsigned index = 0; index < index_map->Length(); index++) {
			Handle<Value> field_index(index_map->Get(NanNew<Integer>(index)));

			if (!field_index->IsUint32()) {
				delete [] index_map_ptr;
				NanThrowError("index map must contain only integer values");
				NanReturnUndefined();
			}

			int val = (int)field_index->Uint32Value(); //todo: validate index? perhaps ogr already does this and throws an error

			index_map_ptr[index] = val;
		}

		err = feature->this_->SetFrom(other_feature->this_, index_map_ptr, forgiving ? TRUE : FALSE);

		delete [] index_map_ptr;
	}

	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}

/**
 * @readOnly
 * @attribute fields
 * @type {gdal.FeatureFields}
 */
NAN_GETTER(Feature::fieldsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("fields_")));
}

/**
 * @attribute fid
 * @type {Number}
 */
NAN_GETTER(Feature::fidGetter)
{
	NanScope();
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(NanNew<Number>(feature->this_->GetFID()));
}

/**
 * @readOnly
 * @attribute defn
 * @type {gdal.FeatureDefn}
 */
NAN_GETTER(Feature::defnGetter)
{
	NanScope();
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}
	NanReturnValue(FeatureDefn::New(feature->this_->GetDefnRef(), false));
}

NAN_SETTER(Feature::fidSetter)
{
	NanScope();
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		NanThrowError("Feature object already destroyed");
		return;
	}
	if(!value->IsInt32()){
		NanThrowError("fid must be an integer");
		return;
	}
	feature->this_->SetFID(value->IntegerValue());
}

} // namespace node_gdal