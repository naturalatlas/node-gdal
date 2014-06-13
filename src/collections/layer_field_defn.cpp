#include "../ogr_common.hpp"
#include "../ogr_field_defn.hpp"
#include "../ogr_layer.hpp"
#include "layer_field_defn.hpp"

Persistent<FunctionTemplate> LayerFieldDefnCollection::constructor;

using namespace node_ogr;

void LayerFieldDefnCollection::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LayerFieldDefnCollection::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LayerFieldDefnCollection"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "indexOf", indexOf);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "alter", alter);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "reorder", reorder);

	ATTR(constructor, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("LayerFieldDefnCollection"), constructor->GetFunction());
}

LayerFieldDefnCollection::LayerFieldDefnCollection()
	: ObjectWrap()
{}

LayerFieldDefnCollection::~LayerFieldDefnCollection() 
{}

Handle<Value> LayerFieldDefnCollection::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		LayerFieldDefnCollection *layer =  static_cast<LayerFieldDefnCollection *>(ptr);
		layer->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create LayerFieldDefnCollection directly");
	}
}

Handle<Value> LayerFieldDefnCollection::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	LayerFieldDefnCollection *wrapped = new LayerFieldDefnCollection();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LayerFieldDefnCollection::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> LayerFieldDefnCollection::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LayerFieldDefnCollection"));
}

Handle<Value> LayerFieldDefnCollection::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		return NODE_THROW("Layer has no layer definition set");
	}

	return scope.Close(Integer::New(def->GetFieldCount()));
}

Handle<Value> LayerFieldDefnCollection::indexOf(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	bool exact = false;
	std::string name("");
	NODE_ARG_STR(0, "field name", name);
	NODE_ARG_BOOL_OPT(1, "exact match", exact);

	return scope.Close(Integer::New(layer->get()->FindFieldIndex(name.c_str(), exact)));
}

Handle<Value> LayerFieldDefnCollection::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	if (args.Length() < 1) {
		return NODE_THROW("Field index or name must be given");
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		return NODE_THROW("Layer has no layer definition set");
	}
	
	int field_index;
	ARG_FIELD_ID(0, def, field_index);

	return scope.Close(FieldDefn::New(def->GetFieldDefn(field_index)));
}

Handle<Value> LayerFieldDefnCollection::getNames(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		return NODE_THROW("Layer has no layer definition set");
	}

	int n = def->GetFieldCount();	
	Handle<Array> result = Array::New(n);

	for (int i = 0; i < n;  i++) {
		OGRFieldDefn *field_def = def->GetFieldDefn(i);
		result->Set(i, SafeString::New(field_def->GetNameRef()));
	}

	return scope.Close(result);
}

Handle<Value> LayerFieldDefnCollection::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}

	if (args.Length() < 1) {
		return NODE_THROW("Field index or name must be given");
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		return NODE_THROW("Layer has no layer definition set");
	}
	
	int field_index;
	ARG_FIELD_ID(0, def, field_index);

	int err = layer->get()->DeleteField(field_index);
	if (err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> LayerFieldDefnCollection::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}
	
	FieldDefn *field_def;
	bool approx = true;
	NODE_ARG_WRAPPED(0, "field definition", FieldDefn, field_def);
	NODE_ARG_BOOL_OPT(1, "approx", approx);

	int err = layer->get()->CreateField(field_def->get(), approx);
	if (err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> LayerFieldDefnCollection::layerGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}