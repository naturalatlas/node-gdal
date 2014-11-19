
#include "gdal_common.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_field_defn.hpp"
#include "collections/feature_defn_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FeatureDefn::constructor;

void FeatureDefn::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(FeatureDefn::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("FeatureDefn"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "clone", clone);

	ATTR(lcons, "name", nameGetter, READ_ONLY_SETTER);
	ATTR(lcons, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "styleIgnored", styleIgnoredGetter, styleIgnoredSetter);
	ATTR(lcons, "geomIgnored", geomIgnoredGetter, geomIgnoredSetter);
	ATTR(lcons, "geomType", geomTypeGetter, geomTypeSetter);

	target->Set(NanNew("FeatureDefn"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

FeatureDefn::FeatureDefn(OGRFeatureDefn *def)
	: ObjectWrap(),
	  this_(def),
	  owned_(true)
{
	LOG("Created FeatureDefn [%p]", def);
}

FeatureDefn::FeatureDefn()
	: ObjectWrap(),
	  this_(0),
	  owned_(true)
{
}

FeatureDefn::~FeatureDefn()
{
	if(this_) {
		LOG("Disposing FeatureDefn [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		if(owned_) this_->Release();
		this_ = NULL;
		LOG("Disposed FeatureDefn [%p]", this_);
	}
}

/**
 * Definition of a feature class or feature layer.
 *
 * @constructor
 * @class gdal.FeatureDefn
 */
NAN_METHOD(FeatureDefn::New)
{
	NanScope();
	FeatureDefn *f;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<FeatureDefn *>(ptr);
	} else {
		if (args.Length() != 0) {
			NanThrowError("FeatureDefn constructor doesn't take any arguments");
			NanReturnUndefined();
		}
		f = new FeatureDefn(new OGRFeatureDefn());
		f->this_->Reference();
	}

	Handle<Value> fields = FeatureDefnFields::New(args.This());
	args.This()->SetHiddenValue(NanNew("fields_"), fields);

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def)
{
	NanEscapableScope();
	return NanEscapeScope(FeatureDefn::New(def, false));
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def, bool owned)
{
	NanEscapableScope();

	if (!def) {
		return NanEscapeScope(NanNull());
	}

	//make a copy of featuredefn owned by a layer
	// + no need to track when a layer is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only featuredefn
	// - is slower

	//TODO: cloning maybe unnecessary if reference counting is done right.
	//      def->Reference(); def->Release();

	if (!owned) {
		def = def->Clone();
	}

	FeatureDefn *wrapped = new FeatureDefn(def);
	wrapped->owned_ = true;
	def->Reference();

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(FeatureDefn::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(FeatureDefn::toString)
{
	NanScope();
	NanReturnValue(NanNew("FeatureDefn"));
}

/**
 * Clones the feature definition.
 *
 * @method clone
 * @return {gdal.FeatureDefn}
 */
NAN_METHOD(FeatureDefn::clone)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	NanReturnValue(FeatureDefn::New(def->this_->Clone()));
}

/**
 * @readOnly
 * @attribute name
 * @type {String}
 */
NAN_GETTER(FeatureDefn::nameGetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	NanReturnValue(SafeString::New(def->this_->GetName()));
}

/**
 * WKB geometry type ({{#crossLink "Constants (wkbGeometryType)"}}see table{{/crossLink}})
 *
 * @attribute geomType
 * @type {Integer}
 */
NAN_GETTER(FeatureDefn::geomTypeGetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	NanReturnValue(NanNew<Integer>(def->this_->GetGeomType()));
}

/**
 * @attribute geomIgnored
 * @type {Boolean}
 */
NAN_GETTER(FeatureDefn::geomIgnoredGetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	NanReturnValue(NanNew<Boolean>(def->this_->IsGeometryIgnored()));
}

/**
 * @attribute styleIgnored
 * @type {Boolean}
 */
NAN_GETTER(FeatureDefn::styleIgnoredGetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	NanReturnValue(NanNew<Boolean>(def->this_->IsStyleIgnored()));
}

/**
 * @readOnly
 * @attribute fields
 * @type {gdal.FeatureDefnFields}
 */
NAN_GETTER(FeatureDefn::fieldsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("fields_")));
}

NAN_SETTER(FeatureDefn::geomTypeSetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	if(!value->IsInt32()){
		NanThrowError("geomType must be an integer");
		return;
	}
	def->this_->SetGeomType(OGRwkbGeometryType(value->IntegerValue()));
}

NAN_SETTER(FeatureDefn::geomIgnoredSetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	if(!value->IsBoolean()){
		NanThrowError("geomIgnored must be a boolean");
		return;
	}
	def->this_->SetGeometryIgnored(value->IntegerValue());
}

NAN_SETTER(FeatureDefn::styleIgnoredSetter)
{
	NanScope();
	FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
	if(!value->IsBoolean()){
		NanThrowError("styleIgnored must be a boolean");
		return;
	}
	def->this_->SetStyleIgnored(value->IntegerValue());
}

} // namespace node_gdal