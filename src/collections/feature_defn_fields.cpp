#include "../gdal_common.hpp"
#include "../gdal_field_defn.hpp"
#include "../gdal_feature_defn.hpp"
#include "feature_defn_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FeatureDefnFields::constructor;

void FeatureDefnFields::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(FeatureDefnFields::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("FeatureDefnFields"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(lcons, "indexOf", indexOf);
	NODE_SET_PROTOTYPE_METHOD(lcons, "reorder", reorder);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "alter", alter);

	ATTR_DONT_ENUM(lcons, "featureDefn", featureDefnGetter, READ_ONLY_SETTER);

	target->Set(NanNew("FeatureDefnFields"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

FeatureDefnFields::FeatureDefnFields()
	: ObjectWrap()
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
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		FeatureDefnFields *feature_def =  static_cast<FeatureDefnFields *>(ptr);
		feature_def->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create FeatureDefnFields directly");
		NanReturnUndefined();
	}
}

Handle<Value> FeatureDefnFields::New(Handle<Value> feature_defn)
{
	NanEscapableScope();

	FeatureDefnFields *wrapped = new FeatureDefnFields();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(FeatureDefnFields::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), feature_defn);

	return NanEscapeScope(obj);
}

NAN_METHOD(FeatureDefnFields::toString)
{
	NanScope();
	NanReturnValue(NanNew("FeatureDefnFields"));
}

/**
 * Returns the number of fields.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(FeatureDefnFields::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(NanNew<Integer>(feature_def->get()->GetFieldCount()));
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	NanReturnValue(NanNew<Integer>(feature_def->get()->GetFieldIndex(name.c_str())));
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() < 1) {
		NanThrowError("Field index or name must be given");
		NanReturnUndefined();
	}

	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	NanReturnValue(FieldDefn::New(feature_def->get()->GetFieldDefn(field_index)));
}

/**
 * Returns a list of field names.
 *
 * @method getNames
 * @return {Array} List of field names.
 */
NAN_METHOD(FeatureDefnFields::getNames)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	int n = feature_def->get()->GetFieldCount();
	Handle<Array> result = NanNew<Array>(n);

	for (int i = 0; i < n;  i++) {
		OGRFieldDefn *field_def = feature_def->get()->GetFieldDefn(i);
		result->Set(i, SafeString::New(field_def->GetNameRef()));
	}

	NanReturnValue(result);
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() < 1) {
		NanThrowError("Field index or name must be given");
		NanReturnUndefined();
	}

	int field_index;
	ARG_FIELD_ID(0, feature_def->get(), field_index);

	int err = feature_def->get()->DeleteFieldDefn(field_index);
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}
	if (args.Length() < 1) {
		NanThrowError("field definition(s) must be given");
		NanReturnUndefined();
	}

	FieldDefn *field_def;

	if (args[0]->IsArray()) {
		Handle<Array> array = args[0].As<Array>();
		int n = array->Length();
		for (int i = 0; i < n; i++) {
			Handle<Value> element = array->Get(i);
			if (IS_WRAPPED(element, FieldDefn)) {
				field_def = ObjectWrap::Unwrap<FieldDefn>(element.As<Object>());
				feature_def->get()->AddFieldDefn(field_def->get());
			} else {
				NanThrowError("All array elements must be FieldDefn objects");
				NanReturnUndefined();
			}
		}
	} else if (IS_WRAPPED(args[0], FieldDefn)) {
		field_def = ObjectWrap::Unwrap<FieldDefn>(args[0].As<Object>());
		feature_def->get()->AddFieldDefn(field_def->get());
	} else {
		NanThrowError("field definition(s) must be a FieldDefn object or array of FieldDefn objects");
		NanReturnUndefined();
	}

	NanReturnUndefined();
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(parent);
	if (!feature_def->get()) {
		NanThrowError("FeatureDefn object already destroyed");
		NanReturnUndefined();
	}

	Handle<Array> field_map = NanNew<Array>(0);
	NODE_ARG_ARRAY(0, "field map", field_map);

	int n = feature_def->get()->GetFieldCount();
	OGRErr err = 0;

	if ((int)field_map->Length() != n) {
		NanThrowError("Array length must match field count");
		NanReturnUndefined();
	}

	int *field_map_array = new int[n];

	for (int i = 0; i < n; i++) {
		Handle<Value> val = field_map->Get(i);
		if (!val->IsNumber()) {
			delete [] field_map_array;
			NanThrowError("Array must only contain integers");
			NanReturnUndefined();
		}

		int key = val->IntegerValue();
		if (key < 0 || key >= n) {
			delete [] field_map_array;
			NanThrowError("Values must be between 0 and field count - 1");
			NanReturnUndefined();
		}

		field_map_array[i] = key;
	}

	err = feature_def->get()->ReorderFieldDefns(field_map_array);

	delete [] field_map_array;

	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
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
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal