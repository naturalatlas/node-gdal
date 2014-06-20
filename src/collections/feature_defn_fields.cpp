#include "../gdal_common.hpp"
#include "../gdal_field_defn.hpp"
#include "../gdal_feature_defn.hpp"
#include "feature_defn_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FeatureDefnFields::constructor;

void FeatureDefnFields::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureDefnFields::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FeatureDefnFields"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "indexOf", indexOf);
	NODE_SET_PROTOTYPE_METHOD(constructor, "reorder", reorder);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "alter", alter);

	ATTR(constructor, "featureDefn", featureDefnGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("FeatureDefnFields"), constructor->GetFunction());
}

FeatureDefnFields::FeatureDefnFields()
	: ObjectWrap()
{}

FeatureDefnFields::~FeatureDefnFields() 
{}

Handle<Value> FeatureDefnFields::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		FeatureDefnFields *feature_def =  static_cast<FeatureDefnFields *>(ptr);
		feature_def->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create FeatureDefnFields directly");
	}
}

Handle<Value> FeatureDefnFields::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	FeatureDefnFields *wrapped = new FeatureDefnFields();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = FeatureDefnFields::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> FeatureDefnFields::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("FeatureDefnFields"));
}

Handle<Value> FeatureDefnFields::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		return NODE_THROW("FeatureDefn object already destroyed");
	}

	return scope.Close(Integer::New(feature_def->get()->GetFieldCount()));
}

Handle<Value> FeatureDefnFields::indexOf(const Arguments& args)
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

Handle<Value> FeatureDefnFields::get(const Arguments& args)
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

Handle<Value> FeatureDefnFields::getNames(const Arguments& args)
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

Handle<Value> FeatureDefnFields::remove(const Arguments& args)
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

Handle<Value> FeatureDefnFields::add(const Arguments& args)
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

Handle<Value> FeatureDefnFields::reorder(const Arguments& args)
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


Handle<Value> FeatureDefnFields::featureDefnGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}

} // namespace node_gdal