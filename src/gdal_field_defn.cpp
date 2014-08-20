
#include "gdal_common.hpp"
#include "gdal_field_defn.hpp"
#include "field_types.hpp"

namespace node_gdal {

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

FieldDefn::FieldDefn(OGRFieldDefn *def)
	: ObjectWrap(),
	  this_(def),
	  owned_(false)
{
	LOG("Created FieldDefn [%p]", def);
}

FieldDefn::FieldDefn()
	: ObjectWrap(),
	  this_(0),
	  owned_(false)
{
}

FieldDefn::~FieldDefn()
{
	if(this_){
		LOG("Disposing FieldDefn [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if(owned_) delete this_;
		LOG("Disposed FieldDefn [%p]", this_);
		this_ = NULL;
	}
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
		std::string type_name("string");

		NODE_ARG_STR(0, "field name", field_name);
		NODE_ARG_STR(1, "field type", type_name);

		int field_type = getFieldTypeByName(type_name);
		if (field_type < 0) {
			return NODE_THROW("Unrecognized field type");
		}

		FieldDefn* def = new FieldDefn(new OGRFieldDefn(field_name.c_str(), static_cast<OGRFieldType>(field_type)));
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
	return scope.Close(SafeString::New(getFieldTypeName(def->this_->GetType())));
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
	OGRJustification justification = def->this_->GetJustify();
	if (justification == OJRight) return scope.Close(String::New("Right"));
	if (justification == OJLeft)  return scope.Close(String::New("Left"));
	return scope.Close(Undefined());
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
	std::string name = TOSTR(value);
	def->this_->SetName(name.c_str());
}

void FieldDefn::typeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsString()){
		NODE_THROW("type must be a string");
		return;
	}
	std::string name = TOSTR(value);
	int type = getFieldTypeByName(name);
	if(type < 0){
		NODE_THROW("Unrecognized field type");
	} else {
		def->this_->SetType(OGRFieldType(type));
	}
	
}

void FieldDefn::justificationSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(info.This());
	

	OGRJustification justification;
	std::string str = TOSTR(value);
	if(value->IsString()){
		if(str == "Left") {
			justification = OJLeft;
		} else if (str == "Right") {
			justification = OJRight;
		} else if (str == "Undefined") {
			justification = OJUndefined; 
		} else {
			NODE_THROW("Unrecognized justification");
			return;
		}
	} else if (value->IsNull() || value->IsUndefined()){
		justification = OJUndefined;
	} else {
		NODE_THROW("justification must be a string or undefined");
		return;
	}
	
	def->this_->SetJustify(justification);
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

} // namespace node_gdal