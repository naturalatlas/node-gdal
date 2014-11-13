#include "../gdal_common.hpp"
#include "../gdal_layer.hpp"
#include "../gdal_feature.hpp"
#include "layer_features.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LayerFeatures::constructor;

void LayerFeatures::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(LayerFeatures::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("LayerFeatures"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "set", set);
	NODE_SET_PROTOTYPE_METHOD(lcons, "first", first);
	NODE_SET_PROTOTYPE_METHOD(lcons, "next", next);
	NODE_SET_PROTOTYPE_METHOD(lcons, "remove", remove);

	ATTR_DONT_ENUM(lcons, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(NanNew("LayerFeatures"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

LayerFeatures::LayerFeatures()
	: ObjectWrap()
{}

LayerFeatures::~LayerFeatures()
{}

/**
 * @class gdal.LayerFeatures
 */
NAN_METHOD(LayerFeatures::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		LayerFeatures *f =  static_cast<LayerFeatures *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create LayerFeatures directly");
		NanReturnUndefined();
	}
}

Handle<Value> LayerFeatures::New(Handle<Value> layer_obj)
{
	NanEscapableScope();

	LayerFeatures *wrapped = new LayerFeatures();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(LayerFeatures::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), layer_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(LayerFeatures::toString)
{
	NanScope();
	NanReturnValue(NanNew("LayerFeatures"));
}

NAN_METHOD(LayerFeatures::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	int feature_id;
	NODE_ARG_INT(0, "feature id", feature_id);
	OGRFeature *feature = layer->get()->GetFeature(feature_id);

	NanReturnValue(Feature::New(feature));
}

NAN_METHOD(LayerFeatures::first)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	layer->get()->ResetReading();
	OGRFeature *feature = layer->get()->GetNextFeature();

	NanReturnValue(Feature::New(feature));
}

NAN_METHOD(LayerFeatures::next)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	OGRFeature *feature = layer->get()->GetNextFeature();

	NanReturnValue(Feature::New(feature));
}

NAN_METHOD(LayerFeatures::add)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	Feature *f;
	NODE_ARG_WRAPPED(0, "feature", Feature, f)

	int err = layer->get()->CreateFeature(f->get());
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}

NAN_METHOD(LayerFeatures::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	int force = 1;
	NODE_ARG_BOOL_OPT(0, "force", force);

	NanReturnValue(NanNew<Integer>(layer->get()->GetFeatureCount(force)));
}

NAN_METHOD(LayerFeatures::set)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	int err;
	Feature *f;
	int argc = args.Length();

	if(argc == 1) {
		NODE_ARG_WRAPPED(0, "feature", Feature, f);
	} else if(argc == 2) {
		int i = 0;
		NODE_ARG_INT(0, "feature id", i);
		NODE_ARG_WRAPPED(1, "feature", Feature, f);
		err = f->get()->SetFID(i);
		if(err) {
			NanThrowError("Error setting feature id");
			NanReturnUndefined();
		}
	} else {
		NanThrowError("Invalid number of arguments");
		NanReturnUndefined();
	}

	if(!f->get()){
		NanThrowError("Feature already destroyed");
		NanReturnUndefined();
	}
	err = layer->get()->SetFeature(f->get());
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}


NAN_METHOD(LayerFeatures::remove)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	int i;
	NODE_ARG_INT(0, "feature id", i);
	int err = layer->get()->DeleteFeature(i);
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_GETTER(LayerFeatures::layerGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal