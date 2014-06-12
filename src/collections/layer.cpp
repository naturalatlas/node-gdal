#include "../ogr_common.hpp"
#include "../ogr_datasource.hpp"
#include "../ogr_layer.hpp"
#include "layer.hpp"

Persistent<FunctionTemplate> LayerCollection::constructor;

void LayerCollection::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LayerCollection::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LayerCollection"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copy", copy);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
}

LayerCollection::LayerCollection()
	: ObjectWrap()
{}

LayerCollection::~LayerCollection() 
{}

Handle<Value> LayerCollection::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		LayerCollection *f =  static_cast<LayerCollection *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create LayerCollection directly");
	}
}

Handle<Value> LayerCollection::New(Handle<Value> ds_obj)
{
	HandleScope scope;

	LayerCollection *wrapped = new LayerCollection();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LayerCollection::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), ds_obj);

	return scope.Close(obj);
}

Handle<Value> LayerCollection::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LayerCollection"));
}

Handle<Value> LayerCollection::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(parent);
	if (!ds->get()) {
		return NODE_THROW("Datasource object already destroyed");
	}

	if(args.Length() < 1) {
		return NODE_THROW("method must be given integer or string")
	}

	OGRLayer *lyr;
	
	if(args[0]->IsString()) {
		lyr = ds->get()->GetLayerByName(TOSTR(args[0]));
	} else if(args[0]->IsNumber()) {
		lyr = ds->get()->GetLayer(args[0]->IntegerValue());
	} else {
		return NODE_THROW("method must be given integer or string")
	}

	return scope.Close(node_ogr::Layer::New(lyr, ds->get()));
}

Handle<Value> LayerCollection::create(const Arguments& args)
{
	HandleScope scope;
	std::string layer_name;
	std::string spatial_ref = "";
	OGRwkbGeometryType geom_type = wkbUnknown;
	Handle<Array> layer_options = Array::New(0);

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(parent);
	if (!ds->get()) {
		return NODE_THROW("Datasource object already destroyed");
	}

	NODE_ARG_STR(0, "layer name", layer_name);
	NODE_ARG_OPT_STR(1, "spatial reference", spatial_ref);
	NODE_ARG_ENUM_OPT(2, "geometry type", OGRwkbGeometryType, geom_type);
	NODE_ARG_ARRAY_OPT(3, "layer creation options", layer_options);

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = ds->get()->CreateLayer(layer_name.c_str(),
					  NULL,
					  geom_type,
					  options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(node_ogr::Layer::New(layer, ds->get(), false));
	} else {
		return NODE_THROW("Error creating layer");
	}
}

Handle<Value> LayerCollection::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(parent);
	if (!ds->get()) {
		return NODE_THROW("Datasource object already destroyed");
	}
	
	return scope.Close(Integer::New(ds->get()->GetLayerCount()));
}

Handle<Value> LayerCollection::copy(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(parent);
	if (!ds->get()) {
		return NODE_THROW("Datasource object already destroyed");
	}

	node_ogr::Layer *layer_to_copy;
	std::string new_name = "";
	Handle<Array> layer_options = Array::New(0);

	NODE_ARG_WRAPPED(0, "layer to copy", node_ogr::Layer, layer_to_copy);
	NODE_ARG_STR(1, "new layer name", new_name);
	NODE_ARG_ARRAY_OPT(2, "layer creation options", layer_options);

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = ds->get()->CopyLayer(layer_to_copy->get(),
										   new_name.c_str(),
										   options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(node_ogr::Layer::New(layer, ds->get()));
	} else {
		return NODE_THROW("Error copying layer");
	}
}


Handle<Value> LayerCollection::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(parent);
	if (!ds->get()) {
		return NODE_THROW("Datasource object already destroyed");
	}
	
	int i;
	NODE_ARG_INT(0, "layer index", i);
	int err = ds->get()->DeleteLayer(i);
	if(err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}
