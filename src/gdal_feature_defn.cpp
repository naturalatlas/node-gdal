
#include "gdal_common.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_field_defn.hpp"
#include "collections/feature_defn_fields.hpp"

Persistent<FunctionTemplate> FeatureDefn::constructor;

void FeatureDefn::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureDefn::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FeatureDefn"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeomType", getGeomType);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeomType", setGeomType);
	NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
	NODE_SET_PROTOTYPE_METHOD(constructor, "isGeometryIgnored", isGeometryIgnored);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometryIgnored", setGeometryIgnored);
	NODE_SET_PROTOTYPE_METHOD(constructor, "isStyleIgnored", isStyleIgnored);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setStyleIgnored", setStyleIgnored);
	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);

	ATTR(constructor, "fields", fieldsGetter, READ_ONLY_SETTER);
	
	target->Set(String::NewSymbol("FeatureDefn"), constructor->GetFunction());
}

FeatureDefn::FeatureDefn(OGRFeatureDefn *layer)
	: ObjectWrap(),
	  this_(layer),
	  owned_(true)
{}

FeatureDefn::FeatureDefn()
	: ObjectWrap(),
	  this_(0),
	  owned_(true)
{
}

FeatureDefn::~FeatureDefn()
{
	if (owned_ && this_) {
		this_->Release();
	}
	this_ = NULL;
}

Handle<Value> FeatureDefn::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		FeatureDefn *f = static_cast<FeatureDefn *>(ptr);
		f->Wrap(args.This());
	}

	Handle<Value> fields = FeatureDefnFields::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("fields_"), fields);

	return args.This();
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def)
{
	HandleScope scope;
	return scope.Close(FeatureDefn::New(def, false));
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def, bool owned)
{
	HandleScope scope;

	if (!def) {
		return Null();
	}

	//make a copy of featuredefn owned by a layer
	// + no need to track when a layer is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only featuredefn
	// - is slower

	if (!owned) {
		def = def->Clone();
	}

	FeatureDefn *wrapped = new FeatureDefn(def);
	wrapped->owned_ = true;

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = FeatureDefn::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> FeatureDefn::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("FeatureDefn"));
}


NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, getName, SafeString, GetName);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, getGeomType, Integer, GetGeomType);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, clone, FeatureDefn, Clone);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, isGeometryIgnored, Boolean, IsGeometryIgnored);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, isStyleIgnored, Boolean, IsStyleIgnored);
NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(FeatureDefn, setGeomType, SetGeomType, OGRwkbGeometryType, "geometry type");
NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(FeatureDefn, setGeometryIgnored, SetGeometryIgnored, "ignore");
NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(FeatureDefn, setStyleIgnored, SetStyleIgnored, "ignore");

Handle<Value> FeatureDefn::fieldsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("fields_")));
}