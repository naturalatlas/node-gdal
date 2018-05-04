#include "typed_array.hpp"

#include <sstream>

namespace node_gdal {

//https://github.com/joyent/node/issues/4201#issuecomment-9837340

Local<Value> TypedArray::New(GDALDataType type, unsigned int length)  {
	Nan::EscapableHandleScope scope;

	Local<Value> val;
	Local<Function> constructor;
	Local<Object> global = Nan::GetCurrentContext()->Global();

	const char *name;
	switch(type) {
		case GDT_Byte:    name = "Uint8Array";   break;
		case GDT_Int16:   name = "Int16Array";   break;
		case GDT_UInt16:  name = "Uint16Array";  break;
		case GDT_Int32:   name = "Int32Array";   break;
		case GDT_UInt32:  name = "Uint32Array";  break;
		case GDT_Float32: name = "Float32Array"; break;
		case GDT_Float64: name = "Float64Array"; break;
		default: 
			Nan::ThrowError("Unsupported array type"); 
			return scope.Escape(Nan::Undefined());
	}


	// make ArrayBuffer
	val = global->Get(Nan::New("ArrayBuffer").ToLocalChecked());

	if(val.IsEmpty() || !val->IsFunction()) {
		Nan::ThrowError("Error getting ArrayBuffer constructor");
		return scope.Escape(Nan::Undefined());
	}

	constructor = val.As<Function>();
	Local<Value> size = Nan::New<Integer>(length * GDALGetDataTypeSize(type) / 8);
	Local<Value> array_buffer = Nan::NewInstance(constructor, 1, &size).ToLocalChecked();

	if(array_buffer.IsEmpty() || !array_buffer->IsObject()) {
		Nan::ThrowError("Error allocating ArrayBuffer");
		return scope.Escape(Nan::Undefined());
	}


	// make TypedArray
	val = global->Get(Nan::New(name).ToLocalChecked());

	if(val.IsEmpty() || !val->IsFunction()) {
		Nan::ThrowError("Error getting typed array constructor");
		return scope.Escape(Nan::Undefined());
	}

	constructor = val.As<Function>();
	Local<Object> array = Nan::NewInstance(constructor, 1, &array_buffer).ToLocalChecked();

	if(array.IsEmpty() || !array->IsObject()) {
		Nan::ThrowError("Error creating TypedArray");
		return scope.Escape(Nan::Undefined());
	}

	array->Set(Nan::New("_gdal_type").ToLocalChecked(), Nan::New(type));

	return scope.Escape(array);
}

GDALDataType TypedArray::Identify(Local<Object> obj) {
	Nan::HandleScope scope;

	Local<String> sym = Nan::New("_gdal_type").ToLocalChecked();
	if (!Nan::HasOwnProperty(obj, sym).FromMaybe(false)) return GDT_Unknown;
	Local<Value> val = obj->Get(sym);
	if (!val->IsNumber()) return GDT_Unknown;

	return (GDALDataType) val->Int32Value();
}

void* TypedArray::Validate(Local<Object> obj, GDALDataType type, int min_length){
	//validate array
	Nan::HandleScope scope;

	GDALDataType src_type = TypedArray::Identify(obj);
	if(src_type == GDT_Unknown) {
		Nan::ThrowTypeError("Unable to identify GDAL datatype of passed array object");
		return NULL;
	}
	if(src_type != type) {
		std::ostringstream ss;
		ss << "Array type does not match band data type (" 
		   << "input: " << GDALGetDataTypeName(src_type)
		   << ", target: " << GDALGetDataTypeName(type) << ")";

		Nan::ThrowTypeError(ss.str().c_str());
		return NULL;
	}
	switch(type) {
		case GDT_Byte: {
			Nan::TypedArrayContents<GByte> contents(obj);
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_Int16: {
			Nan::TypedArrayContents<GInt16> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_UInt16: {
			Nan::TypedArrayContents<GUInt16> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_Int32: {
			Nan::TypedArrayContents<GInt32> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_UInt32: {
			Nan::TypedArrayContents<GUInt32> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_Float32: {
			Nan::TypedArrayContents<float> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		case GDT_Float64: {
			Nan::TypedArrayContents<double> contents(obj); 
			if(ValidateLength(contents.length(), min_length)) return NULL;
			return *contents;
		}
		default:
			Nan::ThrowError("Unsupported array type"); 
			return NULL;
	}
}
bool TypedArray::ValidateLength(int length, int min_length){
	if(length < min_length) {
		std::ostringstream ss;
		ss << "Array length must be greater than or equal to " << min_length; 

		Nan::ThrowError(ss.str().c_str());
		return true;
	}
	return false;
}

} //node_gdal namespace