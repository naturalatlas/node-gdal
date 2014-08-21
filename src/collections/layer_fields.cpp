#include "../gdal_common.hpp"
#include "../gdal_field_defn.hpp"
#include "../gdal_layer.hpp"
#include "layer_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LayerFields::constructor;

void LayerFields::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LayerFields::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LayerFields"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "indexOf", indexOf);
	NODE_SET_PROTOTYPE_METHOD(constructor, "reorder", reorder);
	NODE_SET_PROTOTYPE_METHOD(constructor, "add", add);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "alter", alter);

	ATTR_DONT_ENUM(constructor, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("LayerFields"), constructor->GetFunction());
}

LayerFields::LayerFields()
	: ObjectWrap()
{}

LayerFields::~LayerFields()
{}

Handle<Value> LayerFields::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		LayerFields *layer =  static_cast<LayerFields *>(ptr);
		layer->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create LayerFields directly");
	}
}

Handle<Value> LayerFields::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	LayerFields *wrapped = new LayerFields();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LayerFields::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> LayerFields::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LayerFields"));
}

Handle<Value> LayerFields::count(const Arguments& args)
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

Handle<Value> LayerFields::indexOf(const Arguments& args)
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

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	return scope.Close(Integer::New(def->GetFieldIndex(name.c_str())));
}

Handle<Value> LayerFields::get(const Arguments& args)
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

Handle<Value> LayerFields::getNames(const Arguments& args)
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

Handle<Value> LayerFields::remove(const Arguments& args)
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

Handle<Value> LayerFields::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		return NODE_THROW("Layer object already destroyed");
	}
	if (args.Length() < 1) {
		return NODE_THROW("field definition(s) must be given");
	}

	FieldDefn *field_def;
	int err;
	int approx = 1;
	NODE_ARG_BOOL_OPT(1, "approx", approx);

	if (args[0]->IsArray()) {
		Handle<Array> array = Handle<Array>::Cast(args[0]);
		int n = array->Length();
		for (int i = 0; i < n; i++) {
			Handle<Value> element = array->Get(i);
			if (IS_WRAPPED(element, FieldDefn)) {
				field_def = ObjectWrap::Unwrap<FieldDefn>(element->ToObject());
				err = layer->get()->CreateField(field_def->get(), approx);
				if (err) {
					return NODE_THROW_OGRERR(err);
				}
			} else {
				return NODE_THROW("All array elements must be FieldDefn objects");
			}
		}
	} else if (IS_WRAPPED(args[0], FieldDefn)) {
		field_def = ObjectWrap::Unwrap<FieldDefn>(args[0]->ToObject());
		err = layer->get()->CreateField(field_def->get(), approx);
		if (err) {
			return NODE_THROW_OGRERR(err);
		}
	} else {
		return NODE_THROW("field definition(s) must be a FieldDefn object or array of FieldDefn objects");
	}

	return Undefined();
}

Handle<Value> LayerFields::reorder(const Arguments& args)
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

	Handle<Array> field_map = Array::New(0);
	NODE_ARG_ARRAY(0, "field map", field_map);

	int n = def->GetFieldCount();
	OGRErr err = 0;

	if ((int)field_map->Length() != n) {
		return NODE_THROW("Array length must match field count");
	}

	int *field_map_array = new int[n];

	for (int i = 0; i < n; i++) {
		Handle<Value> val = field_map->Get(i);
		if (!val->IsNumber()) {
			delete [] field_map_array;
			return NODE_THROW("Array must only contain integers");
		}

		int key = val->IntegerValue();
		if (key < 0 || key >= n) {
			delete [] field_map_array;
			return NODE_THROW("Values must be between 0 and field count - 1");
		}

		field_map_array[i] = key;
	}

	err = layer->get()->ReorderFields(field_map_array);

	delete [] field_map_array;

	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}

Handle<Value> LayerFields::layerGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}

} // namespace node_gdal