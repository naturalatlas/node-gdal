#include "../gdal_common.hpp"
#include "../gdal_feature.hpp"
#include "../utils/fast_buffer.hpp"
#include "feature_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> FeatureFields::constructor;

void FeatureFields::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(FeatureFields::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("FeatureFields"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toObject", toObject);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toArray", toArray);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(lcons, "set", set);
	NODE_SET_PROTOTYPE_METHOD(lcons, "reset", reset);
	NODE_SET_PROTOTYPE_METHOD(lcons, "indexOf", indexOf);

	ATTR_DONT_ENUM(lcons, "feature", featureGetter, READ_ONLY_SETTER);

	target->Set(NanNew("FeatureFields"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

FeatureFields::FeatureFields()
	: ObjectWrap()
{}

FeatureFields::~FeatureFields()
{}

/**
 * An encapsulation of all field data that makes up a {{#crossLink "gdal.Feature"}}Feature{{/crossLink}}.
 *
 * @class gdal.FeatureFields
 */
NAN_METHOD(FeatureFields::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		FeatureFields *f =  static_cast<FeatureFields *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create FeatureFields directly");
		NanReturnUndefined();
	}
}

Handle<Value> FeatureFields::New(Handle<Value> layer_obj)
{
	NanEscapableScope();

	FeatureFields *wrapped = new FeatureFields();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(FeatureFields::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), layer_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(FeatureFields::toString)
{
	NanScope();
	NanReturnValue(NanNew("FeatureFields"));
}

inline bool setField(OGRFeature* f, int field_index, Handle<Value> val){
	if (val->IsInt32()) {
		f->SetField(field_index, val->Int32Value());
	} else if (val->IsNumber()) {
		f->SetField(field_index, val->NumberValue());
	} else if (val->IsString()) {
		std::string str = *NanUtf8String(val);
		f->SetField(field_index, str.c_str());
	} else if(val->IsNull() || val->IsUndefined()) {
		f->UnsetField(field_index);
	} else {
		return true;
	}
	return false;
}

/**
 * Sets feature field(s).
 *
 * @example
 * ```
 * // most-efficient, least flexible. requires you to know the ordering of the fields:
 * feature.fields.set(['Something']);
 * feature.fields.set(0, 'Something');
 *
 * // most flexible.
 * feature.fields.set({name: 'Something'});
 * feature.fields.set('name', 'Something');
 * ```
 *
 * @method set
 * @throws Error
 * @param {String|Integer} key Field name or index
 * @param {mixed} value
 */
NAN_METHOD(FeatureFields::set)
{
	NanScope();
	int field_index;
	unsigned int i, n, n_fields_set;

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	if(args.Length() == 1) {
		if(args[0]->IsArray()) {
			//set([])
			Handle<Array> values = args[0].As<Array>();

			n = f->get()->GetFieldCount();
			if(values->Length() < n) {
				n = values->Length();
			}

			for (i = 0; i < n; i++) {
				Handle<Value> val = values->Get(i);
				if(setField(f->get(), i, val)){
					NanThrowError("Unsupported type of field value");
					NanReturnUndefined();
				}
			}

			NanReturnValue(NanNew<Integer>(n));
		} else if (args[0]->IsObject()) {
			//set({})
			Handle<Object> values = args[0].As<Object>();

			n = f->get()->GetFieldCount();
			n_fields_set = 0;

			for (i = 0; i < n; i++) {
				//iterate through field names from field defn,
				//grabbing values from passed object, if not undefined

				OGRFieldDefn* field_def = f->get()->GetFieldDefnRef(i);

				const char* field_name = field_def->GetNameRef();

				field_index = f->get()->GetFieldIndex(field_name);

				//skip value if field name doesnt exist
				//both in the feature definition and the passed object
				if (field_index == -1 || !values->HasOwnProperty(NanNew(field_name))) {
					continue;
				}

				Handle<Value> val = values->Get(NanNew(field_name));
				if (setField(f->get(), field_index, val)) {
					NanThrowError("Unsupported type of field value");
					NanReturnUndefined();
				}

				n_fields_set++;
			}

			NanReturnValue(NanNew<Integer>(n_fields_set));
		} else {
			NanThrowError("Method expected an object or array");
			NanReturnUndefined();
		}

	} else if(args.Length() == 2) {
		//set(name|index, value)
		ARG_FIELD_ID(0, f->get(), field_index);

		//set field value
		if (setField(f->get(), field_index, args[1])) {
			NanThrowError("Unsupported type of field value");
			NanReturnUndefined();
		}

		NanReturnValue(NanNew<Integer>(1));
	} else {
		NanThrowError("Invalid number of arguments");
		NanReturnUndefined();
	}
}

/**
 * Resets all fields.
 *
 * @example
 * ```
 * feature.fields.reset();```
 *
 * @method reset
 * @throws Error
 * @param {Object} [values]
 * @param {mixed} value
 */
NAN_METHOD(FeatureFields::reset)
{
	NanScope();
	int field_index;
	unsigned int i, n;

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	n = f->get()->GetFieldCount();

	if (args.Length() == 0) {
		for (i = 0; i < n; i++) {
			f->get()->UnsetField(i);
		}
		NanReturnValue(NanNew<Integer>(n));
	}

	if (!args[0]->IsObject()) {
		NanThrowError("fields must be an object");
		NanReturnUndefined();
	}

	Handle<Object> values = args[0].As<Object>();

	for (i = 0; i < n; i++) {
		//iterate through field names from field defn,
		//grabbing values from passed object

		OGRFieldDefn* field_def = f->get()->GetFieldDefnRef(i);

		const char* field_name = field_def->GetNameRef();

		field_index = f->get()->GetFieldIndex(field_name);
		if(field_index == -1) continue;

		Handle<Value> val = values->Get(NanNew(field_name));
		if(setField(f->get(), field_index, val)){
			NanThrowError("Unsupported type of field value");
			NanReturnUndefined();
		}
	}

	NanReturnValue(NanNew<Integer>(n));
}

/**
 * Returns the number of fields.
 *
 * @example
 * ```
 * feature.fields.count();```
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(FeatureFields::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(NanNew<Integer>(f->get()->GetFieldCount()));
}

/**
 * Returns the index of a field, given its name.
 *
 * @example
 * ```
 * var index = feature.fields.indexOf('field');```
 *
 * @method indexOf
 * @param {String} name
 * @return {Integer} Index or, `-1` if it cannot be found.
 */
NAN_METHOD(FeatureFields::indexOf)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	NanReturnValue(NanNew<Integer>(f->get()->GetFieldIndex(name.c_str())));
}

/**
 * Outputs the field data as a pure JS object.
 *
 * @throws Error
 * @method toObject
 * @return {Object}
 */
NAN_METHOD(FeatureFields::toObject)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	Local<Object> obj = NanNew<Object>();

	int n = f->get()->GetFieldCount();
	for(int i = 0; i < n; i++) {

		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *key = field_def->GetNameRef();
		if (!key) {
			NanThrowError("Error getting field name");
			NanReturnUndefined();
		}

		//get field value
		Handle<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			NanReturnUndefined(); //get method threw an exception
		}

		obj->Set(NanNew(key), val);
	}
	NanReturnValue(obj);
}

/**
 * Outputs the field values as a pure JS array.
 *
 * @throws Error
 * @method toArray
 * @return {Array}
 */
NAN_METHOD(FeatureFields::toArray)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	int n = f->get()->GetFieldCount();
	Handle<Array> array = NanNew<Array>(n);

	for(int i = 0; i < n; i++) {
		//get field value
		Handle<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			NanReturnUndefined(); //get method threw an exception
		}

		array->Set(i, val);
	}
	NanReturnValue(array);
}

Handle<Value> FeatureFields::get(OGRFeature *f, int field_index)
{
	//#throws : caller must check if return_val.IsEmpty() and bail out if true
	NanEscapableScope();

	if(!f->IsFieldSet(field_index)) return NanEscapeScope(NanNull());

	OGRFieldDefn *field_def = f->GetFieldDefnRef(field_index);
	switch(field_def->GetType()) {
		case OFTInteger:
			return NanEscapeScope(NanNew<Integer>(f->GetFieldAsInteger(field_index)));
		case OFTReal:
			return NanEscapeScope(NanNew<Number>(f->GetFieldAsDouble(field_index)));
		case OFTString:
			return NanEscapeScope(SafeString::New(f->GetFieldAsString(field_index)));
		case OFTIntegerList:
			return NanEscapeScope(getFieldAsIntegerList(f, field_index));
		case OFTRealList:
			return NanEscapeScope(getFieldAsDoubleList(f, field_index));
		case OFTStringList:
			return NanEscapeScope(getFieldAsStringList(f, field_index));
		case OFTBinary:
			return NanEscapeScope(getFieldAsBinary(f, field_index));
		case OFTDate:
		case OFTTime:
		case OFTDateTime:
			return NanEscapeScope(getFieldAsDateTime(f, field_index));
		default:
			NanThrowError("Unsupported field type");
			return NanEscapeScope(NanUndefined());
	}
}

/**
 * Returns a field's value.
 *
 * @example
 * ```
 * value = feature.fields.get(0);
 * value = feature.fields.get('field');```
 *
 * @method get
 * @param {String|Integer} key Feature name or index.
 * @return {mixed|Undefined}
 */
NAN_METHOD(FeatureFields::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() < 1) {
		NanThrowError("Field index or name must be given");
		NanReturnUndefined();
	}

	int field_index;
	ARG_FIELD_ID(0, f->get(), field_index);

	Handle<Value> result = FeatureFields::get(f->get(), field_index);

	if(result.IsEmpty()) {
		NanReturnUndefined();
	} else {
		NanReturnValue(result);
	}
}

/**
 * Returns a list of field name.
 *
 * @method getNames
 * @throws Error
 * @return {Array} List of field names.
 */
NAN_METHOD(FeatureFields::getNames)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		NanThrowError("Feature object already destroyed");
		NanReturnUndefined();
	}

	int n = f->get()->GetFieldCount();
	Handle<Array> result = NanNew<Array>(n);

	for(int i = 0; i < n; i++) {

		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *field_name = field_def->GetNameRef();
		if (!field_name) {
			NanThrowError("Error getting field name");
			NanReturnUndefined();
		}
		result->Set(i, NanNew(field_name));
	}

	NanReturnValue(result);
}

Handle<Value> FeatureFields::getFieldAsIntegerList(OGRFeature* feature, int field_index)
{
	NanEscapableScope();

	int count_of_values = 0;

	const int *values = feature->GetFieldAsIntegerList(field_index, &count_of_values);

	Local<Array> return_array = NanNew<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, NanNew<Integer>(values[index]));
	}

	return NanEscapeScope(return_array);
}


Handle<Value> FeatureFields::getFieldAsDoubleList(OGRFeature* feature, int field_index)
{
	NanEscapableScope();

	int count_of_values = 0;

	const double *values = feature->GetFieldAsDoubleList(field_index, &count_of_values);

	Local<Array> return_array = NanNew<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, NanNew<Number>(values[index]));
	}

	return NanEscapeScope(return_array);
}


Handle<Value> FeatureFields::getFieldAsStringList(OGRFeature* feature, int field_index)
{
	NanEscapableScope();
	char **values = feature->GetFieldAsStringList(field_index);

	int count_of_values = CSLCount(values);

	Local<Array> return_array = NanNew<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, SafeString::New(values[index]));
	}

	return NanEscapeScope(return_array);
}


Handle<Value> FeatureFields::getFieldAsBinary(OGRFeature* feature, int field_index)
{
	NanEscapableScope();

	int count_of_bytes = 0;

	unsigned char *data = (unsigned char*) feature->GetFieldAsBinary(field_index, &count_of_bytes);

	if (count_of_bytes > 0) {
		return NanEscapeScope(FastBuffer::New(data, count_of_bytes));
	}

	return NanEscapeScope(NanUndefined());
}


Handle<Value> FeatureFields::getFieldAsDateTime(OGRFeature* feature, int field_index)
{
	NanEscapableScope();

	int year, month, day, hour, minute, second, timezone;

	year = month = day = hour = minute = second = timezone = 0;

	int result = feature->GetFieldAsDateTime(field_index, &year, &month,
				 &day, &hour, &minute, &second, &timezone);

	if (result == TRUE) {
		Local<Object> hash = NanNew<Object>();

		if (year) {
			hash->Set(NanNew("year"), NanNew<Integer>(year));
		}
		if (month) {
			hash->Set(NanNew("month"), NanNew<Integer>(month));
		}
		if (day) {
			hash->Set(NanNew("day"), NanNew<Integer>(day));
		}
		if (hour) {
			hash->Set(NanNew("hour"), NanNew<Integer>(hour));
		}
		if (minute) {
			hash->Set(NanNew("minute"), NanNew<Integer>(minute));
		}
		if (second) {
			hash->Set(NanNew("second"), NanNew<Integer>(second));
		}
		if (timezone) {
			hash->Set(NanNew("timezone"), NanNew<Integer>(timezone));
		}

		return NanEscapeScope(hash);
	} else {
		return NanEscapeScope(NanUndefined());
	}
}

/**
 * Parent feature
 *
 * @readOnly
 * @attribute feature
 * @type {gdal.Feature}
 */
NAN_GETTER(FeatureFields::featureGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal