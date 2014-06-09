
#include "ogr_common.hpp"
#include "ogr_feature.hpp"
#include "ogr_feature_defn.hpp"
#include "ogr_geometry.hpp"
#include "ogr_field_defn.hpp"

// node
#include <node_buffer.h>

using namespace node_ogr;

Persistent<FunctionTemplate> Feature::constructor;

void Feature::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Feature::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Feature"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getDefn", getDefn);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometryDirectly", setGeometryDirectly);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometry", setGeometry);
	// NODE_SET_PROTOTYPE_METHOD(constructor, "stealGeometry", stealGeometry);
	NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
	NODE_SET_PROTOTYPE_METHOD(constructor, "equal", equal);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldCount", getFieldCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldDefn", getFieldDefn);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldIndex", getFieldIndex);
	NODE_SET_PROTOTYPE_METHOD(constructor, "isFieldSet", isFieldSet);
	NODE_SET_PROTOTYPE_METHOD(constructor, "unsetField", unsetField);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsInteger", getFieldAsInteger);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsDouble", getFieldAsDouble);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsString", getFieldAsString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsIntegerList", getFieldAsIntegerList);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsDoubleList", getFieldAsDoubleList);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsStringList", getFieldAsStringList);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsBinary", getFieldAsBinary);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldAsDateTime", getFieldAsDateTime);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getField", getField);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setField", setField);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFID", getFID);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setFID", setFID);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setFrom", setFrom);
	NODE_SET_PROTOTYPE_METHOD(constructor, "destroy", destroy);

	target->Set(String::NewSymbol("Feature"), constructor->GetFunction());
}

Feature::Feature(OGRFeature *layer)
	: ObjectWrap(),
	  this_(layer),
	  owned_(true)
{}

Feature::Feature()
	: ObjectWrap(),
	  this_(0),
	  owned_(true)
{
}

Feature::~Feature()
{
	dispose();
}

void Feature::dispose()
{
	if (owned_ && this_) {
		OGRFeature::DestroyFeature(this_);
	}
	this_ = NULL;
}


Handle<Value> Feature::New(const Arguments& args)
{
	HandleScope scope;
	Feature* f;

	if (!args.IsConstructCall()) {
		return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<Feature *>(ptr);

	} else {
		FeatureDefn *def;
		NODE_ARG_WRAPPED(0, "feature definition", FeatureDefn, def);
		OGRFeature *ogr_f = new OGRFeature(def->get());
		f = new Feature(ogr_f);
	}

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> Feature::New(OGRFeature *feature)
{
	HandleScope scope;
	return scope.Close(Feature::New(feature, true));
}

Handle<Value> Feature::New(OGRFeature *feature, bool owned)
{
	HandleScope scope;

	if (!feature) {
		return Null();
	}

	Feature *wrapped = new Feature(feature);
	wrapped->owned_ = owned;
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Feature::constructor->GetFunction()->NewInstance(1, &ext);
	return scope.Close(obj);
}

Handle<Value> Feature::toString(const Arguments& args)
{
	return HandleScope().Close(String::New("Feature"));
}

Handle<Value> Feature::getGeometry(const Arguments& args)
{
	HandleScope scope;

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	OGRGeometry* geom = feature->this_->GetGeometryRef();
	if (!geom) {
		NODE_THROW("Error getting feature geometry");
	}


	return scope.Close(Geometry::New(geom, false));
}

Handle<Value> Feature::getDefn(const Arguments& args)
{
	HandleScope scope;

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	return scope.Close(FeatureDefn::New(feature->this_->GetDefnRef(), false));
}

Handle<Value> Feature::getFieldDefn(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(FieldDefn::New(feature->this_->GetFieldDefnRef(field_index), false));
}
//NODE_WRAPPED_METHOD_WITH_RESULT(Feature, stealGeometry, Geometry, StealGeometry);
NODE_WRAPPED_METHOD_WITH_RESULT(Feature, clone, Feature, Clone);
NODE_WRAPPED_METHOD_WITH_RESULT(Feature, getFieldCount, Integer, GetFieldCount);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Feature, setGeometry, SetGeometry, Geometry, "geometry");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Feature, equal, Boolean, Equal, Feature, "feature");
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(Feature, getFieldIndex, Integer, GetFieldIndex, "field name");
NODE_WRAPPED_METHOD_WITH_RESULT(Feature, getFID, Integer, GetFID);
NODE_WRAPPED_METHOD_WITH_1_INTEGER_PARAM(Feature, setFID, SetFID, "feature identifier");

Handle<Value> Feature::destroy(const Arguments& args)
{
	HandleScope scope;
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}
	feature->dispose();
	return Undefined();
}

Handle<Value> Feature::setField(const Arguments& args)
{
	HandleScope scope;
	int field_index;

	if (args.Length() < 2) {
		return NODE_THROW("A value must be specified");
	}

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	//allows field to be referred to by name like python binding
	if (args[0]->IsString()) {
		field_index = feature->this_->GetFieldIndex("name");
		if (field_index == -1) {
			return NODE_THROW("Specified field name does not exist");
		}
	} else if (args[0]->IsInt32()) {
		field_index = args[0]->Int32Value();
		if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
			return NODE_THROW("Invalid field index");
		}
	} else {
		return NODE_THROW("Field index must be integer or string");
	}

	if (args[1]->IsInt32()) {
		feature->this_->SetField(field_index, args[1]->Int32Value());
	} else if (args[1]->IsNumber()) {
		feature->this_->SetField(field_index, args[1]->NumberValue());
	} else if (args[1]->IsString()) {
		feature->this_->SetField(field_index, TOSTR(args[1]));
	} else {
		return NODE_THROW("Invalid value specified to setField");
	}

	return Undefined();
}



Handle<Value> Feature::setGeometryDirectly(const Arguments& args)
{
	//- currently disabled
	//- uses setGeometry instead in case future versions support unowned geometry
	//Geometry objects in V8 must be the owner of the OGRGeometry in the current implementation
	//   to avoid geometry being destroyed when a feature is destroyed

	HandleScope scope;
	return scope.Close(Feature::setGeometry(args));

	/*
	Geometry *geom;
	NODE_ARG_WRAPPED(0, "geometry", Geometry, geom);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) return NODE_THROW("Feature object already destroyed");

	OGRErr err = feature->this_->SetGeometryDirectly(geom->get());

	if (err) return NODE_THROW_OGRERR(err);

	geom->owned_ = false;
	return Undefined();
	*/
}

Handle<Value> Feature::setFrom(const Arguments& args)
{
	HandleScope scope;
	Feature *other_feature;
	bool forgiving = true;
	Handle<Array> index_map;
	OGRErr err;

	NODE_ARG_WRAPPED(0, "feature", Feature, other_feature);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (args.Length() <= 2) {
		NODE_ARG_BOOL_OPT(1, "forgiving", forgiving);

		err = feature->this_->SetFrom(other_feature->this_, forgiving ? TRUE : FALSE);
	} else if (args.Length() > 2) {
		NODE_ARG_ARRAY(1, "index map", index_map);
		NODE_ARG_BOOL_OPT(2, "forgiving", forgiving);

		if (index_map->Length() < 1) {
			return NODE_THROW("index map must contain at least 1 index");
		}

		int *index_map_ptr = new int[index_map->Length()];

		for (unsigned index = 0; index < index_map->Length(); index++) {
			Handle<Value> field_index(index_map->Get(Integer::New(index)));

			if (!field_index->IsUint32()) {
				delete [] index_map_ptr;
				return NODE_THROW("index map must contain only integer values");
			}

			int val = (int)field_index->Uint32Value(); //todo: validate index? perhaps ogr already does this and throws an error

			index_map_ptr[index] = val;
		}

		err = feature->this_->SetFrom(other_feature->this_, index_map_ptr, forgiving ? TRUE : FALSE);

		delete [] index_map_ptr;
	}

	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}

Handle<Value> Feature::isFieldSet(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(Boolean::New(feature->this_->IsFieldSet(field_index)));
}

Handle<Value> Feature::getFieldAsInteger(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(Integer::New(feature->this_->GetFieldAsInteger(field_index)));
}

Handle<Value> Feature::getFieldAsDouble(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(Number::New(feature->this_->GetFieldAsDouble(field_index)));
}

Handle<Value> Feature::getFieldAsString(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(SafeString::New(feature->this_->GetFieldAsString(field_index)));
}

Handle<Value> Feature::getFieldAsIntegerList(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	int count_of_values = 0;

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	const int *values = feature->this_->GetFieldAsIntegerList(field_index, &count_of_values);

	if (count_of_values < 0) {
		return NODE_THROW("Invalid list length");
	}

	Local<Array> return_array = Array::New(count_of_values);

	if (count_of_values > 0) {
		for (int index = 0; index < count_of_values; index++) {
			return_array->Set(index, Integer::New(values[index]));
		}
	}

	return scope.Close(return_array);
}


Handle<Value> Feature::getFieldAsDoubleList(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	int count_of_values = 0;

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	const double *values = feature->this_->GetFieldAsDoubleList(field_index, &count_of_values);

	if (count_of_values < 0) {
		return NODE_THROW("Invalid list length");
	}

	Local<Array> return_array = Array::New(count_of_values);

	if (count_of_values > 0) {
		for (int index = 0; index < count_of_values; index++) {
			return_array->Set(index, Number::New(values[index]));
		}
	}

	return scope.Close(return_array);
}


Handle<Value> Feature::getFieldAsStringList(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	char **values = feature->this_->GetFieldAsStringList(field_index);

	int count_of_values = CSLCount(values);

	if (count_of_values < 0) {
		return NODE_THROW("Invalid list length");
	}

	Local<Array> return_array = Array::New(count_of_values);

	if (count_of_values > 0) {
		for (int index = 0; index < count_of_values; index++) {
			return_array->Set(index, SafeString::New(values[index]));
		}
	}

	return scope.Close(return_array);
}


Handle<Value> Feature::getFieldAsBinary(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	int count_of_bytes = 0;

	GByte *values = feature->this_->GetFieldAsBinary(field_index, &count_of_bytes);

	if (count_of_bytes < 0) {
		return NODE_THROW("Invalid buffer length");
	}

	if (count_of_bytes > 0) {
		char *data = new char[count_of_bytes];
		memcpy(data, values, count_of_bytes);
		Local<Buffer> return_buffer = Buffer::New(data, count_of_bytes);
		return scope.Close(return_buffer->handle_);
	}

	return Undefined();
}


Handle<Value> Feature::getFieldAsDateTime(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	int year, month, day, hour, minute, second, timezone;

	year = month = day = hour = minute = second = timezone = 0;

	int result = feature->this_->GetFieldAsDateTime(field_index, &year, &month,
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
		return NODE_THROW("Could not get field as DateTime object");
	}
}

Handle<Value> Feature::getField(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	OGRFieldDefn *field_def = feature->this_->GetFieldDefnRef(field_index);
	switch(field_def->GetType()) {
	case OFTInteger:
		return feature->getFieldAsInteger(args);
	case OFTIntegerList:
		return feature->getFieldAsIntegerList(args);
	case OFTReal:
		return feature->getFieldAsDouble(args);
	case OFTRealList:
		return feature->getFieldAsDoubleList(args);
	case OFTString:
		return feature->getFieldAsString(args);
	case OFTStringList:
		return feature->getFieldAsStringList(args);
	case OFTBinary:
		return feature->getFieldAsBinary(args);
	case OFTDate:
		return feature->getFieldAsDateTime(args);
	case OFTTime:
		return feature->getFieldAsDateTime(args);
	case OFTDateTime:
		return feature->getFieldAsDateTime(args);
	default:
		return NODE_THROW("Unsupported field type in getFieldValue method");
	}
}

Handle<Value> Feature::unsetField(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	feature->this_->UnsetField(field_index);
	return Undefined();
}