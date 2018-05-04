#include "../gdal_common.hpp"
#include "../gdal_field_defn.hpp"
#include "../gdal_feature_defn.hpp"
#include "feature_defn_fields.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> FeatureDefnFields::constructor;

void FeatureDefnFields::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(FeatureDefnFields::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("FeatureDefnFields").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "remove", remove);
	Nan::SetPrototypeMethod(lcons, "getNames", getNames);
	Nan::SetPrototypeMethod(lcons, "indexOf", indexOf);
	Nan::SetPrototypeMethod(lcons, "reorder", reorder);
	Nan::SetPrototypeMethod(lcons, "add", add);
	//Nan::SetPrototypeMethod(lcons, "alter", alter);

	ATTR_DONT_ENUM(lcons, "featureDefn", featureDefnGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("FeatureDefnFields").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

FeatureDefnFields::FeatureDefnFields()
	: Nan::ObjectWrap()
{}

FeatureDefnFields::~FeatureDefnFields()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.FeatureDefn"}}FeatureDefn{{/crossLink}}'s fields.
 *
 * @class gdal.FeatureDefnFields
 */
NAN_METHOD(FeatureDefnFields::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		FeatureDefnFields *feature_def =  static_cast<FeatureDefnFields *>(ptr);
		feature_def->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create FeatureDefnFields directly");
		return;
	}
}

Local<Value> FeatureDefnFields::New(Local<Value> feature_defn)
{
	Nan::EscapableHandleScope scope;

	FeatureDefnFields *wrapped = new FeatureDefnFields();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(FeatureDefnFields::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), feature_defn);

	return scope.Escape(obj);
}

NAN_METHOD(FeatureDefnFields::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("FeatureDefnFields").ToLocalChecked());
}

/**
 * Returns the number of fields.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(FeatureDefnFields::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	info.GetReturnValue().Set(Nan::New<Integer>(feature_def->get()->GetFieldCount()));
}

/**
 * Returns the index of field definition.
 *
 * @method indexOf
 * @param {String} name
 * @return {Integer} Index or `-1` if not found.
 */
NAN_METHOD(FeatureDefnFields::indexOf)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	info.GetReturnValue().Set(Nan::New<Integer>(feature_def->get()->GetFieldIndex(name.c_str())));
}

/**
 * Returns a field definition.
 *
 * @method get
 * @param {String|Integer} key Field name or index
 * @return {gdal.FieldDefn}
 */
NAN_METHOD(FeatureDefnFields::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	if (info.Length() < 1) {
		Nan::ThrowError("Field index or name must be given");
		return;
	}

	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	info.GetReturnValue().Set(FieldDefn::New(feature_def->get()->GetFieldDefn(field_index)));
}

/**
 * Returns a list of field names.
 *
 * @method getNames
 * @return {Array} List of field names.
 */
NAN_METHOD(FeatureDefnFields::getNames)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	int n = feature_def->get()->GetFieldCount();
	Local<Array> result = Nan::New<Array>(n);

	for (int i = 0; i < n;  i++) {
		OGRFieldDefn *field_def = feature_def->get()->GetFieldDefn(i);
		result->Set(i, SafeString::New(field_def->GetNameRef()));
	}

	info.GetReturnValue().Set(result);
}

/**
 * Removes a field definition.
 *
 * @method remove
 * @throws Error
 * @param {String|Integer} key Field name or index
 */
NAN_METHOD(FeatureDefnFields::remove)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	if (info.Length() < 1) {
		Nan::ThrowError("Field index or name must be given");
		return;
	}

	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	int err = feature_def->get()->DeleteFieldDefn(field_index);
	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	return;
}

/**
 * Adds field definition(s).
 *
 * @method add
 * @throws Error
 * @param {gdal.FieldDefn|Array} field(s)
 */
NAN_METHOD(FeatureDefnFields::add)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}
	if (info.Length() < 1) {
		Nan::ThrowError("field definition(s) must be given");
		return;
	}

	FieldDefn *field_def;

	if (info[0]->IsArray()) {
		Local<Array> array = info[0].As<Array>();
		int n = array->Length();
		for (int i = 0; i < n; i++) {
			Local<Value> element = array->Get(i);
			if (IS_WRAPPED(element, FieldDefn)) {
				field_def = Nan::ObjectWrap::Unwrap<FieldDefn>(element.As<Object>());
				feature_def->get()->AddFieldDefn(field_def->get());
			} else {
				Nan::ThrowError("All array elements must be FieldDefn objects");
				return;
			}
		}
	} else if (IS_WRAPPED(info[0], FieldDefn)) {
		field_def = Nan::ObjectWrap::Unwrap<FieldDefn>(info[0].As<Object>());
		feature_def->get()->AddFieldDefn(field_def->get());
	} else {
		Nan::ThrowError("field definition(s) must be a FieldDefn object or array of FieldDefn objects");
		return;
	}

	return;
}

/**
 * Reorders the fields.
 *
 * @example
 * ```
 * // reverse fields:
 * featureDef.fields.reorder([2, 1, 0]);```
 *
 * @method reorder
 * @throws Error
 * @param {Array} map An array representing the new field order.
 */
NAN_METHOD(FeatureDefnFields::reorder)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	FeatureDefn *feature_def = Nan::ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->isAlive()) {
		Nan::ThrowError("FeatureDefn object already destroyed");
		return;
	}

	Local<Array> field_map = Nan::New<Array>(0);
	NODE_ARG_ARRAY(0, "field map", field_map);

	int n = feature_def->get()->GetFieldCount();
	OGRErr err = 0;

	if ((int)field_map->Length() != n) {
		Nan::ThrowError("Array length must match field count");
		return;
	}

	int *field_map_array = new int[n];

	for (int i = 0; i < n; i++) {
		Local<Value> val = field_map->Get(i);
		if (!val->IsNumber()) {
			delete [] field_map_array;
			Nan::ThrowError("Array must only contain integers");
			return;
		}

		int key = val->IntegerValue();
		if (key < 0 || key >= n) {
			delete [] field_map_array;
			Nan::ThrowError("Values must be between 0 and field count - 1");
			return;
		}

		field_map_array[i] = key;
	}

	err = feature_def->get()->ReorderFieldDefns(field_map_array);

	delete [] field_map_array;

	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	return;
}

/**
 * Parent feature definition.
 *
 * @readOnly
 * @attribute featureDefn
 * @type {gdal.FeatureDefn}
 */
NAN_GETTER(FeatureDefnFields::featureDefnGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
