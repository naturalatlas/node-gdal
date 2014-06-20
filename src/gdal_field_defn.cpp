
#include "gdal_common.hpp"
#include "gdal_field_defn.hpp"

Persistent<FunctionTemplate> FieldDefn::constructor;

void FieldDefn::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FieldDefn::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FieldDefn"));

	ATTR(constructor, "name", nameGetter, nameSetter);
	ATTR(constructor, "type", typeGetter, typeSetter);
	ATTR(constructor, "justification", justificationGetter, justificationSetter);
	ATTR(constructor, "width", widthGetter, widthSetter);
	ATTR(constructor, "precision", precisionGetter, precisionSetter);
	ATTR(constructor, "ignored", ignoredGetter, ignoredSetter);

	target->Set(String::NewSymbol("FieldDefn"), constructor->GetFunction());
}

FieldDefn::FieldDefn(OGRFieldDefn *layer)
	: ObjectWrap(),
	  this_(layer),
	  owned_(false)
{}

FieldDefn::FieldDefn()
	: ObjectWrap(),
	  this_(0),
	  owned_(false)
{
}

FieldDefn::~FieldDefn()
{
	if (owned_ && this_) {
		delete this_;
	}
	this_ = NULL;
}

Handle<Value> FieldDefn::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		FieldDefn *f = static_cast<FieldDefn *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		std::string field_name("");
		OGRFieldType field_type(OFTString);

		NODE_ARG_STR(0, "field name", field_name);
		NODE_ARG_ENUM(1, "field type", OGRFieldType, field_type);

		FieldDefn* def = new FieldDefn(new OGRFieldDefn(field_name.c_str(), field_type));
		def->owned_ = true;
		def->Wrap(args.This());
	}

	return args.This();
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def)
{
	HandleScope scope;
	return scope.Close(FieldDefn::New(def, false));
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def, bool owned)
{
	HandleScope scope;

	if (!def) {
		return Null();
	}

	//make a copy of fielddefn owned by a featuredefn
	// + no need to track when a featuredefn is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only fielddefn
	// - is slower

	if (!owned) {
		def = new OGRFieldDefn(def);
	}

	FieldDefn *wrapped = new FieldDefn(def);
	wrapped->owned_ = true;

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = FieldDefn::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> FieldDefn::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("FieldDefn"));
}


Handle<Value> FieldDefn::nameGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(SafeString::New(def->this_->GetNameRef()));
}

Handle<Value> FieldDefn::typeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(Integer::New(def->this_->GetType()));
}

Handle<Value> FieldDefn::ignoredGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(Boolean::New(def->this_->IsIgnored()));
}

Handle<Value> FieldDefn::justificationGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(Integer::New(def->this_->GetJustify()));
}

Handle<Value> FieldDefn::widthGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(Integer::New(def->this_->GetWidth()));
}

Handle<Value> FieldDefn::precisionGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	return scope.Close(Integer::New(def->this_->GetPrecision()));
}

void FieldDefn::nameSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsString()){
		NODE_THROW("Name must be string");
		return;
	}
	def->this_->SetName(TOSTR(value));
}

void FieldDefn::typeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		NODE_THROW("type must be an integer");
		return;
	}
	def->this_->SetType(OGRFieldType(value->IntegerValue()));
}

void FieldDefn::justificationSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		NODE_THROW("justification must be an integer");
		return;
	}
	def->this_->SetJustify(OGRJustification(value->IntegerValue()));
}

void FieldDefn::widthSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		NODE_THROW("width must be an integer");
		return;
	}
	def->this_->SetWidth(value->IntegerValue());
}

void FieldDefn::precisionSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		NODE_THROW("precision must be an integer");
		return;
	}
	def->this_->SetPrecision(value->IntegerValue());
}

void FieldDefn::ignoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsBoolean()){
		NODE_THROW("ignored must be a boolean");
		return;
	}
	def->this_->SetIgnored(value->BooleanValue());
}