#include "../ogr_common.hpp"
#include "../ogr_field_defn.hpp"
#include "../ogr_feature_defn.hpp"
#include "field_defn.hpp"

Persistent<FunctionTemplate> FieldDefnCollection::constructor;

using namespace node_ogr;

void FieldDefnCollection::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FieldDefnCollection::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FieldDefnCollection"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "indexOf", indexOf);
	NODE_SET_PROTOTYPE_METHOD(constructor, "reorder", reorder);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "alter", alter);

	ATTR(constructor, "featureDefn", featureDefnGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("FieldDefnCollection"), constructor->GetFunction());
}

FieldDefnCollection::FieldDefnCollection()
	: ObjectWrap()
{}

FieldDefnCollection::~FieldDefnCollection() 
{}

Handle<Value> FieldDefnCollection::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		FieldDefnCollection *feature_def =  static_cast<FieldDefnCollection *>(ptr);
		feature_def->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create FieldDefnCollection directly");
	}
}

Handle<Value> FieldDefnCollection::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	FieldDefnCollection *wrapped = new FieldDefnCollection();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = FieldDefnCollection::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> FieldDefnCollection::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("FieldDefnCollection"));
}

Handle<Value> FieldDefnCollection::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	return scope.Close(Integer::New(feature_def->get()->GetFieldCount()));
}

Handle<Value> FieldDefnCollection::indexOf(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	return scope.Close(Integer::New(feature_def->get()->GetFieldIndex(name.c_str())));
}

Handle<Value> FieldDefnCollection::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	if (args.Length() < 1) {
		return NODE_THROW("Field index or name must be given");
	}
	
	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	return scope.Close(FieldDefn::New(feature_def->get()->GetFieldDefn(field_index)));
}

Handle<Value> FieldDefnCollection::getNames(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	int n = feature_def->get()->GetFieldCount();	
	Handle<Array> result = Array::New(n);

	for (int i = 0; i < n;  i++) {
		OGRFieldDefn *field_def = feature_def->get()->GetFieldDefn(i);
		result->Set(i, SafeString::New(field_def->GetNameRef()));
	}

	return scope.Close(result);
}

Handle<Value> FieldDefnCollection::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	if (args.Length() < 1) {
		return NODE_THROW("Field index or name must be given");
	}
	
	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	int err = feature_def->get()->DeleteFieldDefn(field_index);
	if (err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> FieldDefnCollection::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}
	
	FieldDefn *field_def;
	NODE_ARG_WRAPPED(0, "field definition", FieldDefn, field_def);

	feature_def->get()->AddFieldDefn(field_def->get());

	return Undefined();
}

Handle<Value> FieldDefnCollection::reorder(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	Handle<Array> field_map = Array::New(0);
	NODE_ARG_ARRAY(0, "field map", field_map);

	int n = feature_def->get()->GetFieldCount();
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
	
	err = feature_def->get()->ReorderFieldDefns(field_map_array);

	delete [] field_map_array;

	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}


Handle<Value> FieldDefnCollection::featureDefnGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}