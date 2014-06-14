#include "../ogr_common.hpp"
#include "../ogr_feature.hpp"
#include "feature_fields.hpp"

// node
#include <node_buffer.h>

Persistent<FunctionTemplate> FeatureFields::constructor;

using namespace node_ogr;

void FeatureFields::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureFields::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("FeatureFields"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "toJSON", toJSON);
	NODE_SET_PROTOTYPE_METHOD(constructor, "toArray", toArray);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "set", set);
	NODE_SET_PROTOTYPE_METHOD(constructor, "reset", reset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "indexOf", indexOf);

	ATTR(constructor, "feature", featureGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("FeatureFields"), constructor->GetFunction());
}

FeatureFields::FeatureFields()
	: ObjectWrap()
{}

FeatureFields::~FeatureFields() 
{}

Handle<Value> FeatureFields::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		FeatureFields *f =  static_cast<FeatureFields *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create FeatureFields directly");
	}
}

Handle<Value> FeatureFields::New(Handle<Value> layer_obj)
{
	HandleScope scope;

	FeatureFields *wrapped = new FeatureFields();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = FeatureFields::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), layer_obj);

	return scope.Close(obj);
}

Handle<Value> FeatureFields::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("FeatureFields"));
}


inline bool setField(OGRFeature* f, int field_index, Handle<Value> val){
	if (val->IsInt32()) {
		f->SetField(field_index, val->Int32Value());
	} else if (val->IsNumber()) {
		f->SetField(field_index, val->NumberValue());
	} else if (val->IsString()) {
		f->SetField(field_index, TOSTR(val));
	} else if(val->IsNull() || val->IsUndefined()) {
		f->UnsetField(field_index);
	} else {
		return true;
	}
	return false;
}

Handle<Value> FeatureFields::set(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	unsigned int i, n, n_fields_set;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}
	
	if(args.Length() == 1) {
		if(args[0]->IsArray()) {
			//set([]) 
			Handle<Array> values = Handle<Array>::Cast(args[0]);

			n = f->get()->GetFieldCount();
			if(values->Length() < n) {
				n = values->Length();
			}

			for (i = 0; i < n; i++) {
				Handle<Value> val = values->Get(i);
				if(setField(f->get(), i, val)){
					return NODE_THROW("Unsupported type of field value");
				}
			}

			return scope.Close(Integer::New(n));
		} else if (args[0]->IsObject()) {
			//set({}) 
			Handle<Object> values = Handle<Object>::Cast(args[0]);
		
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
				if (field_index == -1 || !values->HasOwnProperty(String::NewSymbol(field_name))) {
					continue;
				}

				Handle<Value> val = values->Get(String::NewSymbol(field_name));
				if (setField(f->get(), field_index, val)) {
					return NODE_THROW("Unsupported type of field value");
				}

				n_fields_set++;
			}

			return scope.Close(Integer::New(n_fields_set));
		} else {
			return NODE_THROW("Method expected an object or array");
		}

	} else if(args.Length() == 2) {
		//set(name|index, value)
		ARG_FIELD_ID(0, f->get(), field_index);

		//set field value
		if (setField(f->get(), field_index, args[1])) {
			return NODE_THROW("Unsupported type of field value");
		}

		return scope.Close(Integer::New(1));	
	} else {
		return NODE_THROW("Invalid number of arguments")
	}
}

Handle<Value> FeatureFields::reset(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	unsigned int i, n;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}
	
	Handle<Object> obj;
	NODE_ARG_OBJECT(0, "fields", obj);

	Handle<Object> values = Handle<Object>::Cast(args[0]);
		
	n = f->get()->GetFieldCount();
	
	for (i = 0; i < n; i++) {
		//iterate through field names from field defn,
		//grabbing values from passed object
		 
		OGRFieldDefn* field_def = f->get()->GetFieldDefnRef(i);
		
		const char* field_name = field_def->GetNameRef();
		
		field_index = f->get()->GetFieldIndex(field_name);
		if(field_index == -1) continue;

		Handle<Value> val = values->Get(String::NewSymbol(field_name));
		if(setField(f->get(), field_index, val)){
			return NODE_THROW("Unsupported type of field value");
		}
	}

	return scope.Close(Integer::New(n));
}

Handle<Value> FeatureFields::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	return scope.Close(Integer::New(f->get()->GetFieldCount()));
}

Handle<Value> FeatureFields::indexOf(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	return scope.Close(Integer::New(f->get()->GetFieldIndex(name.c_str())));
}

Handle<Value> FeatureFields::toJSON(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	Local<Object> obj = Object::New();

	int n = f->get()->GetFieldCount();
	for(int i = 0; i < n; i++) {
		
		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *key = field_def->GetNameRef();
		if (!key) {
			return NODE_THROW("Error getting field name");
		} 

		//get field value
		Handle<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			return val; //get method threw an exception	
		}

		obj->Set(String::NewSymbol(key), val);
	}
	return scope.Close(obj);
}

Handle<Value> FeatureFields::toArray(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	int n = f->get()->GetFieldCount();
	Handle<Array> array = Array::New(n);

	for(int i = 0; i < n; i++) {
		//get field value
		Handle<Value> val = FeatureFields::get(f->get(), i);
		if (val.IsEmpty()) {
			return val; //get method threw an exception	
		}
		
		array->Set(i, val);
	}
	return scope.Close(array);
}


Handle<Value> FeatureFields::get(OGRFeature *f, int field_index)
{
	//#throws : caller must check if return_val.IsEmpty() and bail out if true
	HandleScope scope;

	if(!f->IsFieldSet(field_index)) return Null();

	OGRFieldDefn *field_def = f->GetFieldDefnRef(field_index);
	switch(field_def->GetType()) {
		case OFTInteger:
			return scope.Close(Integer::New(f->GetFieldAsInteger(field_index)));
		case OFTReal:
			return scope.Close(Number::New(f->GetFieldAsDouble(field_index)));
		case OFTString:
			return scope.Close(SafeString::New(f->GetFieldAsString(field_index)));
		case OFTIntegerList:
			return scope.Close(getFieldAsIntegerList(f, field_index));
		case OFTRealList:
			return scope.Close(getFieldAsDoubleList(f, field_index));
		case OFTStringList:
			return scope.Close(getFieldAsStringList(f, field_index));
		case OFTBinary:
			return scope.Close(getFieldAsBinary(f, field_index));
		case OFTDate:
		case OFTTime:
		case OFTDateTime:
			return scope.Close(getFieldAsDateTime(f, field_index));
		default:
			return NODE_THROW("Unsupported field type");
	}
}
Handle<Value> FeatureFields::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (args.Length() < 1) {
		return NODE_THROW("Field index or name must be given");
	} 
	
	int field_index;
	ARG_FIELD_ID(0, f->get(), field_index);

	Handle<Value> result = FeatureFields::get(f->get(), field_index);
	
	//check if exception... not sure if this is needed
	if(result.IsEmpty()) return result;
	else return scope.Close(result);
}

Handle<Value> FeatureFields::getNames(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Feature *f = ObjectWrap::Unwrap<Feature>(parent);
	if (!f->get()) {
		return NODE_THROW("Feature object already destroyed");
	}

	int n = f->get()->GetFieldCount();
	Handle<Array> result = Array::New(n);

	for(int i = 0; i < n; i++) {
		
		//get field name
		OGRFieldDefn *field_def = f->get()->GetFieldDefnRef(i);
		const char *field_name = field_def->GetNameRef();
		if (!field_name) {
			return NODE_THROW("Error getting field name");
		}
		result->Set(i, String::New(field_name));
	}

	return scope.Close(result);
}

Handle<Value> FeatureFields::getFieldAsIntegerList(OGRFeature* feature, int field_index)
{
	HandleScope scope;
	
	int count_of_values = 0;

	const int *values = feature->GetFieldAsIntegerList(field_index, &count_of_values);

	Local<Array> return_array = Array::New(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, Integer::New(values[index]));
	}

	return scope.Close(return_array);
}


Handle<Value> FeatureFields::getFieldAsDoubleList(OGRFeature* feature, int field_index)
{
	HandleScope scope;
	
	int count_of_values = 0;

	const double *values = feature->GetFieldAsDoubleList(field_index, &count_of_values);

	Local<Array> return_array = Array::New(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, Number::New(values[index]));
	}

	return scope.Close(return_array);
}


Handle<Value> FeatureFields::getFieldAsStringList(OGRFeature* feature, int field_index)
{
	HandleScope scope;
	char **values = feature->GetFieldAsStringList(field_index);

	int count_of_values = CSLCount(values);

	Local<Array> return_array = Array::New(count_of_values);

	for (int index = 0; index < count_of_values; index++) {
		return_array->Set(index, SafeString::New(values[index]));
	}

	return scope.Close(return_array);
}


Handle<Value> FeatureFields::getFieldAsBinary(OGRFeature* feature, int field_index)
{
	HandleScope scope;
	
	int count_of_bytes = 0;

	GByte *values = feature->GetFieldAsBinary(field_index, &count_of_bytes);

	if (count_of_bytes > 0) {
		char *data = new char[count_of_bytes];
		memcpy(data, values, count_of_bytes);
		Local<Buffer> return_buffer = Buffer::New(data, count_of_bytes);
		return scope.Close(return_buffer->handle_);
	}

	return Undefined();
}


Handle<Value> FeatureFields::getFieldAsDateTime(OGRFeature* feature, int field_index)
{
	HandleScope scope;

	int year, month, day, hour, minute, second, timezone;

	year = month = day = hour = minute = second = timezone = 0;

	int result = feature->GetFieldAsDateTime(field_index, &year, &month,
				 &day, &hour, &minute, &second, &timezone);

	if (result == TRUE) {
		Local<Object> hash = Object::New();

		if (year) {
			hash->Set(String::New("year"), Integer::New(year));
		}
		if (month) {
			hash->Set(String::New("month"), Integer::New(month));
		}
		if (day) {
			hash->Set(String::New("day"), Integer::New(day));
		}
		if (hour) {
			hash->Set(String::New("hour"), Integer::New(hour));
		}
		if (minute) {
			hash->Set(String::New("minute"), Integer::New(minute));
		}
		if (second) {
			hash->Set(String::New("second"), Integer::New(second));
		}
		if (timezone) {
			hash->Set(String::New("timezone"), Integer::New(timezone));
		}

		return scope.Close(hash);
	} else {
		return Undefined();
	}
}

Handle<Value> FeatureFields::featureGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}