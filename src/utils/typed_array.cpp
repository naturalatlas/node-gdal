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
	Local<Value> array_buffer = constructor->NewInstance(1, &size);

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
	Local<Object> array = constructor->NewInstance(1, &array_buffer);

	if(array.IsEmpty() || !array->IsObject()) {
		Nan::ThrowError("Error creating TypedArray");
		return scope.Escape(Nan::Undefined());
	}

	return scope.Escape(array);
}

GDALDataType TypedArray::Identify(Local<Object> obj) {	
	switch(obj->GetIndexedPropertiesExternalArrayDataType()){
		case kExternalByteArray:          return GDT_Byte;
		case kExternalUnsignedByteArray:  return GDT_Byte;
		case kExternalShortArray:         return GDT_Int16;
		case kExternalUnsignedShortArray: return GDT_UInt16;
		case kExternalIntArray:           return GDT_Int32;
		case kExternalUnsignedIntArray:   return GDT_UInt32;
		case kExternalFloatArray:         return GDT_Float32;
		case kExternalDoubleArray:        return GDT_Float64;
		default:                          return GDT_Unknown;
	}
}

void* TypedArray::Validate(Local<Object> obj, GDALDataType type, int min_length){
	//validate array
	Nan::EscapableHandleScope scope;

	if(!obj->HasIndexedPropertiesInExternalArrayData()) {
		Nan::ThrowError("Object has no external array data");
		return NULL;
	}
	GDALDataType src_type = TypedArray::Identify(obj);
	if(type == GDT_Unknown) {
		Nan::ThrowTypeError("Unable to identify GDAL datatype of passed array object");
		return NULL;
	}
	if(src_type != type) {
		std::ostringstream ss;
		ss << "Array type does not match band data type (" 
		   << "array: " << GDALGetDataTypeName(src_type)
		   << " band: " << GDALGetDataTypeName(type) << ")";

		Nan::ThrowTypeError(ss.str().c_str());
		return NULL;
	}
	if(TypedArray::Length(obj) < min_length) {
		std::ostringstream ss;
		ss << "Array length must be greater than or equal to " << min_length; 

		Nan::ThrowError(ss.str().c_str());
		return NULL;
	}
	return TypedArray::Data(obj);
}

int TypedArray::Length(Local<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayDataLength();
}
void* TypedArray::Data(Local<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayData();
}

} //node_gdal namespace