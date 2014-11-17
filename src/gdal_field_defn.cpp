
#include "gdal_common.hpp"
#include "gdal_field_defn.hpp"
#include "utils/field_types.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FieldDefn::constructor;

void FieldDefn::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(FieldDefn::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("FieldDefn"));

	ATTR(lcons, "name", nameGetter, nameSetter);
	ATTR(lcons, "type", typeGetter, typeSetter);
	ATTR(lcons, "justification", justificationGetter, justificationSetter);
	ATTR(lcons, "width", widthGetter, widthSetter);
	ATTR(lcons, "precision", precisionGetter, precisionSetter);
	ATTR(lcons, "ignored", ignoredGetter, ignoredSetter);

	target->Set(NanNew("FieldDefn"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
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


/**
 * @constructor
 * @class gdal.FieldDefn
 * @param {String} name Field name
 * @param {String} type Data type (see {{#crossLink "Constants (OFT)"}}OFT constants{{/crossLink}})
 */
NAN_METHOD(FieldDefn::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		FieldDefn *f = static_cast<FieldDefn *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		std::string field_name("");
		std::string type_name("string");

		NODE_ARG_STR(0, "field name", field_name);
		NODE_ARG_STR(1, "field type", type_name);

		int field_type = getFieldTypeByName(type_name);
		if (field_type < 0) {
			NanThrowError("Unrecognized field type");
			NanReturnUndefined();
		}

		FieldDefn* def = new FieldDefn(new OGRFieldDefn(field_name.c_str(), static_cast<OGRFieldType>(field_type)));
		def->owned_ = true;
		def->Wrap(args.This());
	}

	NanReturnValue(args.This());
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def)
{
	NanEscapableScope();
	return NanEscapeScope(FieldDefn::New(def, false));
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def, bool owned)
{
	NanEscapableScope();

	if (!def) {
		return NanEscapeScope(NanNull());
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

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(FieldDefn::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(FieldDefn::toString)
{
	NanScope();
	NanReturnValue(NanNew("FieldDefn"));
}

/**
 * @attribute name
 * @type {String}
 */
NAN_GETTER(FieldDefn::nameGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	NanReturnValue(SafeString::New(def->this_->GetNameRef()));
}

/**
 * Data type (see {{#crossLink "Constants (OFT)"}}OFT constants{{/crossLink}})
 *
 * @attribute type
 * @type {String}
 */
NAN_GETTER(FieldDefn::typeGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	NanReturnValue(SafeString::New(getFieldTypeName(def->this_->GetType())));
}

/**
 * @attribute ignored
 * @type {Boolean}
 */
NAN_GETTER(FieldDefn::ignoredGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	NanReturnValue(NanNew<Boolean>(def->this_->IsIgnored()));
}

/**
 * Field justification (see {{#crossLink "Constants (OJ)"}}OJ constants{{/crossLink}})
 *
 * @attribute justification
 * @type {String}
 */
NAN_GETTER(FieldDefn::justificationGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	OGRJustification justification = def->this_->GetJustify();
	if (justification == OJRight) NanReturnValue(NanNew("Right"));
	if (justification == OJLeft)  NanReturnValue(NanNew("Left"));
	NanReturnValue(NanUndefined());
}

/**
 * @attribute width
 * @type {Integer}
 */
NAN_GETTER(FieldDefn::widthGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	NanReturnValue(NanNew<Integer>(def->this_->GetWidth()));
}

/**
 * @attribute precision
 * @type {Integer}
 */
NAN_GETTER(FieldDefn::precisionGetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	NanReturnValue(NanNew<Integer>(def->this_->GetPrecision()));
}

NAN_SETTER(FieldDefn::nameSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	if(!value->IsString()){
		NanThrowError("Name must be string");
		return;
	}
	std::string name = *NanUtf8String(value);
	def->this_->SetName(name.c_str());
}

NAN_SETTER(FieldDefn::typeSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	if(!value->IsString()){
		NanThrowError("type must be a string");
		return;
	}
	std::string name = *NanUtf8String(value);
	int type = getFieldTypeByName(name.c_str());
	if(type < 0){
		NanThrowError("Unrecognized field type");
	} else {
		def->this_->SetType(OGRFieldType(type));
	}

}

NAN_SETTER(FieldDefn::justificationSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());


	OGRJustification justification;
	std::string str = *NanUtf8String(value);
	if(value->IsString()){
		if(str == "Left") {
			justification = OJLeft;
		} else if (str == "Right") {
			justification = OJRight;
		} else if (str == "Undefined") {
			justification = OJUndefined;
		} else {
			NanThrowError("Unrecognized justification");
			return;
		}
	} else if (value->IsNull() || value->IsUndefined()){
		justification = OJUndefined;
	} else {
		NanThrowError("justification must be a string or undefined");
		return;
	}

	def->this_->SetJustify(justification);
}

NAN_SETTER(FieldDefn::widthSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	if(!value->IsInt32()){
		NanThrowError("width must be an integer");
		return;
	}
	def->this_->SetWidth(value->IntegerValue());
}

NAN_SETTER(FieldDefn::precisionSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	if(!value->IsInt32()){
		NanThrowError("precision must be an integer");
		return;
	}
	def->this_->SetPrecision(value->IntegerValue());
}

NAN_SETTER(FieldDefn::ignoredSetter)
{
	NanScope();
	FieldDefn *def = ObjectWrap::Unwrap<FieldDefn>(args.This());
	if(!value->IsBoolean()){
		NanThrowError("ignored must be a boolean");
		return;
	}
	def->this_->SetIgnored(value->IntegerValue());
}

} // namespace node_gdal