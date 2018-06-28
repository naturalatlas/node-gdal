#include "../gdal_common.hpp"
#include "../gdal_layer.hpp"
#include "../gdal_feature.hpp"
#include "layer_features.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> LayerFeatures::constructor;

void LayerFeatures::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(LayerFeatures::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("LayerFeatures").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "add", add);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "set", set);
	Nan::SetPrototypeMethod(lcons, "first", first);
	Nan::SetPrototypeMethod(lcons, "next", next);
	Nan::SetPrototypeMethod(lcons, "remove", remove);

	ATTR_DONT_ENUM(lcons, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("LayerFeatures").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

LayerFeatures::LayerFeatures()
	: Nan::ObjectWrap()
{}

LayerFeatures::~LayerFeatures()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.Layer"}}Layer{{/crossLink}}'s features.
 *
 * @class gdal.LayerFeatures
 */
NAN_METHOD(LayerFeatures::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		LayerFeatures *f =  static_cast<LayerFeatures *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create LayerFeatures directly");
		return;
	}
}

Local<Value> LayerFeatures::New(Local<Value> layer_obj)
{
	Nan::EscapableHandleScope scope;

	LayerFeatures *wrapped = new LayerFeatures();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(LayerFeatures::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), layer_obj);

	return scope.Escape(obj);
}

NAN_METHOD(LayerFeatures::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("LayerFeatures").ToLocalChecked());
}

/**
 * Fetch a feature by its identifier.
 *
 * **Important:** The `id` argument is not an index. In most cases it will be zero-based,
 * but in some cases it will not. If iterating, it's best to use the `next()` method.
 *
 * @method get
 * @param {Integer} id The feature ID of the feature to read.
 * @return {gdal.Feature}
 */
NAN_METHOD(LayerFeatures::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	int feature_id;
	NODE_ARG_INT(0, "feature id", feature_id);
	OGRFeature *feature = layer->get()->GetFeature(feature_id);

	info.GetReturnValue().Set(Feature::New(feature));
}

/**
 * Resets the feature pointer used by `next()` and
 * returns the first feature in the layer.
 *
 * @method first
 * @return {gdal.Feature}
 */
NAN_METHOD(LayerFeatures::first)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	layer->get()->ResetReading();
	OGRFeature *feature = layer->get()->GetNextFeature();

	info.GetReturnValue().Set(Feature::New(feature));
}

/**
 * Returns the next feature in the layer. Returns null if no more features.
 *
 * @example
 * ```
 * while (feature = layer.features.next()) { ... }```
 *
 * @method next
 * @return {gdal.Feature}
 */
NAN_METHOD(LayerFeatures::next)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	OGRFeature *feature = layer->get()->GetNextFeature();

	info.GetReturnValue().Set(Feature::New(feature));
}

/**
 * Adds a feature to the layer. The feature should be created using the current layer as the definition.
 *
 * @example
 * ```
 * var feature = new gdal.Feature(layer);
 * feature.setGeometry(new gdal.Point(0, 1));
 * feature.fields.set('name', 'somestring');
 * layer.features.add(feature);```
 *
 * @method add
 * @throws Error
 * @param {gdal.Feature} feature
 */
NAN_METHOD(LayerFeatures::add)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	Feature *f;
	NODE_ARG_WRAPPED(0, "feature", Feature, f)

	int err = layer->get()->CreateFeature(f->get());
	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	return;
}

/**
 * Returns the number of features in the layer.
 *
 * @method count
 * @param {Boolean} [force=true]
 * @return {Integer} Number of features in the layer.
 */
NAN_METHOD(LayerFeatures::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	int force = 1;
	NODE_ARG_BOOL_OPT(0, "force", force);

	info.GetReturnValue().Set(Nan::New<Number>(layer->get()->GetFeatureCount(force)));
}

/**
 * Sets a feature in the layer.
 *
 * @method set
 * @throws Error
 * @param {Integer} [id]
 * @param {gdal.Feature} feature
 */
NAN_METHOD(LayerFeatures::set)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	int err;
	Feature *f;
	int argc = info.Length();

	if(argc == 1) {
		NODE_ARG_WRAPPED(0, "feature", Feature, f);
	} else if(argc == 2) {
		int i = 0;
		NODE_ARG_INT(0, "feature id", i);
		NODE_ARG_WRAPPED(1, "feature", Feature, f);
		err = f->get()->SetFID(i);
		if(err) {
			Nan::ThrowError("Error setting feature id");
			return;
		}
	} else {
		Nan::ThrowError("Invalid number of arguments");
		return;
	}

	if(!f->isAlive()){
		Nan::ThrowError("Feature already destroyed");
		return;
	}
	err = layer->get()->SetFeature(f->get());
	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	return;
}

/**
 * Removes a feature from the layer.
 *
 * @method remove
 * @throws Error
 * @param {Integer} id
 */
NAN_METHOD(LayerFeatures::remove)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Layer *layer = Nan::ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->isAlive()) {
		Nan::ThrowError("Layer object already destroyed");
		return;
	}

	int i;
	NODE_ARG_INT(0, "feature id", i);
	int err = layer->get()->DeleteFeature(i);
	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	return;
}

/**
 * Parent layer
 *
 * @attribute layer
 * @type {gdal.Layer}
 */
NAN_GETTER(LayerFeatures::layerGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
