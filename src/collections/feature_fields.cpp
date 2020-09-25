#include "../gdal_common.hpp"
#include "../gdal_feature.hpp"
#include "feature_fields.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> FeatureFields::constructor;

void FeatureFields::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(FeatureFields::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("FeatureFields").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "toObject", toObject);
	Nan::SetPrototypeMethod(lcons, "toArray", toArray);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "getNames", getNames);
	Nan::SetPrototypeMethod(lcons, "set", set);
	Nan::SetPrototypeMethod(lcons, "reset", reset);
	Nan::SetPrototypeMethod(lcons, "indexOf", indexOf);

	ATTR_DONT_ENUM(lcons, "feature", featureGetter, READ_ONLY_SETTER);

	Nan::Set(target, Nan::New("FeatureFields").ToLocalChecked(), Nan::GetFunction(lcons).ToLocalChecked());

	constructor.Reset(lcons);
}

FeatureFields::FeatureFields()
	: Nan::ObjectWrap()
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
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		FeatureFields *f =  static_cast<FeatureFields *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create FeatureFields directly");
		return;
	}
}

Local<Value> FeatureFields::New(Local<Value> layer_obj)
{
	Nan::EscapableHandleScope scope;

	FeatureFields *wrapped = new FeatureFields();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::GetFunction(Nan::New(FeatureFields::constructor)).ToLocalChecked(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), layer_obj);

	return scope.Escape(obj);
}

NAN_METHOD(FeatureFields::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("FeatureFields").ToLocalChecked());
}

inline bool setField(OGRFeature* f, int field_index, Local<Value> val){
	if (val->IsInt32()) {
		f->SetField(field_index, Nan::To<int32_t>(val).ToChecked());
	} else if (val->IsNumber()) {
		f->SetField(field_index, Nan::To<double>(val).ToChecked());
	} else if (val->IsString()) {
		std::string str = *Nan::Utf8String(val);
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
	Nan::HandleScope scope;
	int field_index;
	unsigned int i, n, n_fields_set;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	if(info.Length() == 1) {
		if(info[0]->IsArray()) {
			//set([])
			Local<Array> values = info[0].As<Array>();

			n = f->get()->GetFieldCount();
			if(values->Length() < n) {
				n = values->Length();
			}

			for (i = 0; i < n; i++) {
				Local<Value> val = Nan::Get(values, i).ToLocalChecked();
				if(setField(f->get(), i, val)){
					Nan::ThrowError("Unsupported type of field value");
					return;
				}
			}

			info.GetReturnValue().Set(Nan::New<Integer>(n));
			return;
		} else if (info[0]->IsObject()) {
			//set({})
			Local<Object> values = info[0].As<Object>();

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
				if (field_index == -1 || !Nan::HasOwnProperty(values, Nan::New(field_name).ToLocalChecked()).FromMaybe(false)) {
					continue;
				}

				Local<Value> val = Nan::Get(values, Nan::New(field_name).ToLocalChecked()).ToLocalChecked();
				if (setField(f->get(), field_index, val)) {
					Nan::ThrowError("Unsupported type of field value");
					return;
				}

				n_fields_set++;
			}

			info.GetReturnValue().Set(Nan::New<Integer>(n_fields_set));
			return;
		} else {
			Nan::ThrowError("Method expected an object or array");
			return;
		}

	} else if(info.Length() == 2) {
		//set(name|index, value)
		ARG_FIELD_ID(0, f->get(), field_index);

		//set field value
		if (setField(f->get(), field_index, info[1])) {
			Nan::ThrowError("Unsupported type of field value");
			return;
		}

		info.GetReturnValue().Set(Nan::New<Integer>(1));
		return;
	} else {
		Nan::ThrowError("Invalid number of arguments");
		return;
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
	Nan::HandleScope scope;
	int field_index;
	unsigned int i, n;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	n = f->get()->GetFieldCount();

	if (info.Length() == 0) {
		for (i = 0; i < n; i++) {
			f->get()->UnsetField(i);
		}
		info.GetReturnValue().Set(Nan::New<Integer>(n));
		return;
	}

	if (!info[0]->IsObject()) {
		Nan::ThrowError("fields must be an object");
		return;
	}

	Local<Object> values = info[0].As<Object>();

	for (i = 0; i < n; i++) {
		//iterate through field names from field defn,
		//grabbing values from passed object

		OGRFieldDefn* field_def = f->get()->GetFieldDefnRef(i);

		const char* field_name = field_def->GetNameRef();

		field_index = f->get()->GetFieldIndex(field_name);
		if(field_index == -1) continue;

		Local<Value> val = Nan::Get(values, Nan::New(field_name).ToLocalChecked()).ToLocalChecked();
		if(setField(f->get(), field_index, val)){
			Nan::ThrowError("Unsupported type of field value");
			return;
		}
	}

	info.GetReturnValue().Set(Nan::New<Integer>(n));
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	info.GetReturnValue().Set(Nan::New<Integer>(f->get()->GetFieldCount()));
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	info.GetReturnValue().Set(Nan::New<Integer>(f->get()->GetFieldIndex(name.c_str())));
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	Local<Object> obj = Nan::New<Object>();

	int n = f->get()->GetFieldCount();
	for(int i = 0; i < n; i++) {

		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *key = field_def->GetNameRef();
		if (!key) {
			Nan::ThrowError("Error getting field name");
			return;
		}

		//get field value
		Local<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			return; //get method threw an exception
		}

		Nan::Set(obj, Nan::New(key).ToLocalChecked(), val);
	}
	info.GetReturnValue().Set(obj);
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	int n = f->get()->GetFieldCount();
	Local<Array> array = Nan::New<Array>(n);

	for(int i = 0; i < n; i++) {
		//get field value
		Local<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			return; //get method threw an exception
		}

		Nan::Set(array, i, val);
	}
	info.GetReturnValue().Set(array);
}

Local<Value> FeatureFields::get(OGRFeature *f, int field_index)
{
	//#throws : caller must check if return_val.IsEmpty() and bail out if true
	Nan::EscapableHandleScope scope;

	if(!f->IsFieldSet(field_index)) return scope.Escape(Nan::Null());

	OGRFieldDefn *field_def = f->GetFieldDefnRef(field_index);
	switch(field_def->GetType()) {
		case OFTInteger:
			return scope.Escape(Nan::New<Integer>(f->GetFieldAsInteger(field_index)));
		#if defined(GDAL_VERSION_MAJOR) && (GDAL_VERSION_MAJOR >= 2)
		case OFTInteger64:
			return scope.Escape(Nan::New<Number>(f->GetFieldAsInteger64(field_index)));
		#endif
		case OFTReal:
			return scope.Escape(Nan::New<Number>(f->GetFieldAsDouble(field_index)));
		case OFTString:
			return scope.Escape(SafeString::New(f->GetFieldAsString(field_index)));
		case OFTIntegerList:
			return scope.Escape(getFieldAsIntegerList(f, field_index));
		case OFTRealList:
			return scope.Escape(getFieldAsDoubleList(f, field_index));
		case OFTStringList:
			return scope.Escape(getFieldAsStringList(f, field_index));
		case OFTBinary:
			return scope.Escape(getFieldAsBinary(f, field_index));
		case OFTDate:
		case OFTTime:
		case OFTDateTime:
			return scope.Escape(getFieldAsDateTime(f, field_index));
		default:
			Nan::ThrowError("Unsupported field type");
			return scope.Escape(Nan::Undefined());
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	if (info.Length() < 1) {
		Nan::ThrowError("Field index or name must be given");
		return;
	}

	int field_index;
	ARG_FIELD_ID(0, f->get(), field_index);

	Local<Value> result = FeatureFields::get(f->get(), field_index);

	if(result.IsEmpty()) {
		return;
	} else {
		info.GetReturnValue().Set(result);
		return;
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Feature *f = Nan::ObjectWrap::Unwrap<Feature>(parent);
	if (!f->isAlive()) {
		Nan::ThrowError("Feature object already destroyed");
		return;
	}

	int n = f->get()->GetFieldCount();
	Local<Array> result = Nan::New<Array>(n);

	for(int i = 0; i < n; i++) {

		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *field_name = field_def->GetNameRef();
		if (!field_name) {
			Nan::ThrowError("Error getting field name");
			return;
		}
		Nan::Set(result, i, Nan::New(field_name).ToLocalChecked());
	}

	info.GetReturnValue().Set(result);
}

Local<Value> FeatureFields::getFieldAsIntegerList(OGRFeature* feature, int field_index)
{
	Nan::EscapableHandleScope scope;

	int count_of_values = 0;

	const int *values = feature->GetFieldAsIntegerList(field_index, &count_of_values);

	Local<Array> return_array = Nan::New<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		Nan::Set(return_array, index, Nan::New<Integer>(values[index]));
	}

	return scope.Escape(return_array);
}


Local<Value> FeatureFields::getFieldAsDoubleList(OGRFeature* feature, int field_index)
{
	Nan::EscapableHandleScope scope;

	int count_of_values = 0;

	const double *values = feature->GetFieldAsDoubleList(field_index, &count_of_values);

	Local<Array> return_array = Nan::New<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		Nan::Set(return_array, index, Nan::New<Number>(values[index]));
	}

	return scope.Escape(return_array);
}


Local<Value> FeatureFields::getFieldAsStringList(OGRFeature* feature, int field_index)
{
	Nan::EscapableHandleScope scope;
	char **values = feature->GetFieldAsStringList(field_index);

	int count_of_values = CSLCount(values);

	Local<Array> return_array = Nan::New<Array>(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		Nan::Set(return_array, index, SafeString::New(values[index]));
	}

	return scope.Escape(return_array);
}


Local<Value> FeatureFields::getFieldAsBinary(OGRFeature* feature, int field_index)
{
	Nan::EscapableHandleScope scope;

	int count_of_bytes = 0;

	char *data = (char *)feature->GetFieldAsBinary(field_index, &count_of_bytes);

	if (count_of_bytes > 0) {
		return scope.Escape(Nan::NewBuffer(data, count_of_bytes).ToLocalChecked());
	}

	return scope.Escape(Nan::Undefined());
}


Local<Value> FeatureFields::getFieldAsDateTime(OGRFeature* feature, int field_index)
{
	Nan::EscapableHandleScope scope;

	int year, month, day, hour, minute, second, timezone;

	year = month = day = hour = minute = second = timezone = 0;

	int result = feature->GetFieldAsDateTime(field_index, &year, &month,
				 &day, &hour, &minute, &second, &timezone);

	if (result == TRUE) {
		Local<Object> hash = Nan::New<Object>();

		if (year) {
			Nan::Set(hash, Nan::New("year").ToLocalChecked(), Nan::New<Integer>(year));
		}
		if (month) {
			Nan::Set(hash, Nan::New("month").ToLocalChecked(), Nan::New<Integer>(month));
		}
		if (day) {
			Nan::Set(hash, Nan::New("day").ToLocalChecked(), Nan::New<Integer>(day));
		}
		if (hour) {
			Nan::Set(hash, Nan::New("hour").ToLocalChecked(), Nan::New<Integer>(hour));
		}
		if (minute) {
			Nan::Set(hash, Nan::New("minute").ToLocalChecked(), Nan::New<Integer>(minute));
		}
		if (second) {
			Nan::Set(hash, Nan::New("second").ToLocalChecked(), Nan::New<Integer>(second));
		}
		if (timezone) {
			Nan::Set(hash, Nan::New("timezone").ToLocalChecked(), Nan::New<Integer>(timezone));
		}

		return scope.Escape(hash);
	} else {
		return scope.Escape(Nan::Undefined());
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
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
