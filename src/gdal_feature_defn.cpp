
#include "gdal_common.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_field_defn.hpp"
#include "collections/feature_defn_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FeatureDefn::constructor;

void FeatureDefn::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureDefn::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FeatureDefn"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);

	ATTR(constructor, "name", nameGetter, READ_ONLY_SETTER);
	ATTR(constructor, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "styleIgnored", styleIgnoredGetter, styleIgnoredSetter);
	ATTR(constructor, "geomIgnored", geomIgnoredGetter, geomIgnoredSetter);
	ATTR(constructor, "geomType", geomTypeGetter, geomTypeSetter);
	
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


NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, clone, FeatureDefn, Clone);

Handle<Value> FeatureDefn::nameGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	return scope.Close(SafeString::New(def->this_->GetName()));
}

Handle<Value> FeatureDefn::geomTypeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	return scope.Close(Integer::New(def->this_->GetGeomType()));
}

Handle<Value> FeatureDefn::geomIgnoredGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	return scope.Close(Boolean::New(def->this_->IsGeometryIgnored()));
}

Handle<Value> FeatureDefn::styleIgnoredGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	return scope.Close(Boolean::New(def->this_->IsStyleIgnored()));
}

Handle<Value> FeatureDefn::fieldsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("fields_")));
}

void FeatureDefn::geomTypeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	if(!value->IsInt32()){
		NODE_THROW("geomType must be an integer");
		return;
	}
	def->this_->SetGeomType(OGRwkbGeometryType(value->IntegerValue()));
}

void FeatureDefn::geomIgnoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	if(!value->IsBoolean()){
		NODE_THROW("geomIgnored must be a boolean");
		return;
	}
	def->this_->SetGeometryIgnored(value->BooleanValue());
}

void FeatureDefn::styleIgnoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(info.This());
	if(!value->IsBoolean()){
		NODE_THROW("styleIgnored must be a boolean");
		return;
	}
	def->this_->SetStyleIgnored(value->BooleanValue());
}

} // namespace node_gdal