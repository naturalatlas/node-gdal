
#include "gdal_common.hpp"
#include "gdal_field_defn.hpp"
#include "utils/field_types.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> FieldDefn::constructor;

void FieldDefn::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(FieldDefn::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("FieldDefn").ToLocalChecked());

	ATTR(lcons, "name", nameGetter, nameSetter);
	ATTR(lcons, "type", typeGetter, typeSetter);
	ATTR(lcons, "justification", justificationGetter, justificationSetter);
	ATTR(lcons, "width", widthGetter, widthSetter);
	ATTR(lcons, "precision", precisionGetter, precisionSetter);
	ATTR(lcons, "ignored", ignoredGetter, ignoredSetter);

	target->Set(Nan::New("FieldDefn").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

FieldDefn::FieldDefn(OGRFieldDefn *def)
	: Nan::ObjectWrap(),
	  this_(def),
	  owned_(false)
{
	LOG("Created FieldDefn [%p]", def);
}

FieldDefn::FieldDefn()
	: Nan::ObjectWrap(),
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


/**
 * @constructor
 * @class gdal.FieldDefn
 * @param {String} name Field name
 * @param {String} type Data type (see {{#crossLink "Constants (OFT)"}}OFT constants{{/crossLink}})
 */
NAN_METHOD(FieldDefn::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		FieldDefn *f = static_cast<FieldDefn *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		std::string field_name("");
		std::string type_name("string");

		NODE_ARG_STR(0, "field name", field_name);
		NODE_ARG_STR(1, "field type", type_name);

		int field_type = getFieldTypeByName(type_name);
		if (field_type < 0) {
			Nan::ThrowError("Unrecognized field type");
			return;
		}

		FieldDefn* def = new FieldDefn(new OGRFieldDefn(field_name.c_str(), static_cast<OGRFieldType>(field_type)));
		def->owned_ = true;
		def->Wrap(info.This());
	}

	info.GetReturnValue().Set(info.This());
}

Local<Value> FieldDefn::New(OGRFieldDefn *def)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(FieldDefn::New(def, false));
}

Local<Value> FieldDefn::New(OGRFieldDefn *def, bool owned)
{
	Nan::EscapableHandleScope scope;

	if (!def) {
		return scope.Escape(Nan::Null());
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

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(FieldDefn::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(FieldDefn::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("FieldDefn").ToLocalChecked());
}

/**
 * @attribute name
 * @type {String}
 */
NAN_GETTER(FieldDefn::nameGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	info.GetReturnValue().Set(SafeString::New(def->this_->GetNameRef()));
}

/**
 * Data type (see {{#crossLink "Constants (OFT)"}}OFT constants{{/crossLink}})
 *
 * @attribute type
 * @type {String}
 */
NAN_GETTER(FieldDefn::typeGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	info.GetReturnValue().Set(SafeString::New(getFieldTypeName(def->this_->GetType())));
}

/**
 * @attribute ignored
 * @type {Boolean}
 */
NAN_GETTER(FieldDefn::ignoredGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	info.GetReturnValue().Set(Nan::New<Boolean>(def->this_->IsIgnored()));
}

/**
 * Field justification (see {{#crossLink "Constants (OJ)"}}OJ constants{{/crossLink}})
 *
 * @attribute justification
 * @type {String}
 */
NAN_GETTER(FieldDefn::justificationGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	OGRJustification justification = def->this_->GetJustify();
	if (justification == OJRight) { 
		info.GetReturnValue().Set(Nan::New("Right").ToLocalChecked());
		return;
	}
	if (justification == OJLeft) {
		info.GetReturnValue().Set(Nan::New("Left").ToLocalChecked());
		return;
	}
	info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * @attribute width
 * @type {Integer}
 */
NAN_GETTER(FieldDefn::widthGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	info.GetReturnValue().Set(Nan::New<Integer>(def->this_->GetWidth()));
}

/**
 * @attribute precision
 * @type {Integer}
 */
NAN_GETTER(FieldDefn::precisionGetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	info.GetReturnValue().Set(Nan::New<Integer>(def->this_->GetPrecision()));
}

NAN_SETTER(FieldDefn::nameSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsString()){
		Nan::ThrowError("Name must be string");
		return;
	}
	std::string name = *Nan::Utf8String(value);
	def->this_->SetName(name.c_str());
}

NAN_SETTER(FieldDefn::typeSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsString()){
		Nan::ThrowError("type must be a string");
		return;
	}
	std::string name = *Nan::Utf8String(value);
	int type = getFieldTypeByName(name.c_str());
	if(type < 0){
		Nan::ThrowError("Unrecognized field type");
	} else {
		def->this_->SetType(OGRFieldType(type));
	}

}

NAN_SETTER(FieldDefn::justificationSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());


	OGRJustification justification;
	std::string str = *Nan::Utf8String(value);
	if(value->IsString()){
		if(str == "Left") {
			justification = OJLeft;
		} else if (str == "Right") {
			justification = OJRight;
		} else if (str == "Undefined") {
			justification = OJUndefined;
		} else {
			Nan::ThrowError("Unrecognized justification");
			return;
		}
	} else if (value->IsNull() || value->IsUndefined()){
		justification = OJUndefined;
	} else {
		Nan::ThrowError("justification must be a string or undefined");
		return;
	}

	def->this_->SetJustify(justification);
}

NAN_SETTER(FieldDefn::widthSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		Nan::ThrowError("width must be an integer");
		return;
	}
	def->this_->SetWidth(value->IntegerValue());
}

NAN_SETTER(FieldDefn::precisionSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsInt32()){
		Nan::ThrowError("precision must be an integer");
		return;
	}
	def->this_->SetPrecision(value->IntegerValue());
}

NAN_SETTER(FieldDefn::ignoredSetter)
{
	Nan::HandleScope scope;
	FieldDefn *def = Nan::ObjectWrap::Unwrap<FieldDefn>(info.This());
	if(!value->IsBoolean()){
		Nan::ThrowError("ignored must be a boolean");
		return;
	}
	def->this_->SetIgnored(value->IntegerValue());
}

} // namespace node_gdal