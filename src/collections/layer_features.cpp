#include "../gdal_common.hpp"
#include "../gdal_layer.hpp"
#include "../gdal_feature.hpp"
#include "layer_features.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LayerFeatures::constructor;

void LayerFeatures::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LayerFeatures::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LayerFeatures"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "add", add);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "set", set);
	NODE_SET_PROTOTYPE_METHOD(constructor, "first", first);
	NODE_SET_PROTOTYPE_METHOD(constructor, "next", next);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);

	ATTR(constructor, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("LayerFeatures"), constructor->GetFunction());
}

LayerFeatures::LayerFeatures()
	: ObjectWrap()
{}

LayerFeatures::~LayerFeatures() 
{}

Handle<Value> LayerFeatures::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		LayerFeatures *f =  static_cast<LayerFeatures *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create LayerFeatures directly");
	}
}

Handle<Value> LayerFeatures::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	LayerFeatures *wrapped = new LayerFeatures();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LayerFeatures::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> LayerFeatures::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LayerFeatures"));
}

Handle<Value> LayerFeatures::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	int feature_id;
	NODE_ARG_INT(0, "feature id", feature_id);
	OGRFeature *feature = layer->get()->GetFeature(feature_id);

	return scope.Close(Feature::New(feature));
}

Handle<Value> LayerFeatures::first(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	layer->get()->ResetReading();
	OGRFeature *feature = layer->get()->GetNextFeature();

	return scope.Close(Feature::New(feature));
}

Handle<Value> LayerFeatures::next(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	OGRFeature *feature = layer->get()->GetNextFeature();

	return scope.Close(Feature::New(feature));
}

Handle<Value> LayerFeatures::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	Feature *f;
	NODE_ARG_WRAPPED(0, "feature", Feature, f)
	
	int err = layer->get()->CreateFeature(f->get());
	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}

Handle<Value> LayerFeatures::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	bool force = true;
	NODE_ARG_BOOL_OPT(0, "force", force);

	return scope.Close(Integer::New(layer->get()->GetFeatureCount(force)));
}

Handle<Value> LayerFeatures::set(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
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
			return NODE_THROW("Error setting feature id");
		}
	} else {
		return NODE_THROW("Invalid number of arguments");
	}

	err = layer->get()->SetFeature(f->get());
	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}


Handle<Value> LayerFeatures::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}
	
	int i;
	NODE_ARG_INT(0, "feature id", i);
	int err = layer->get()->DeleteFeature(i);
	if(err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> LayerFeatures::layerGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}

} // namespace node_gdal