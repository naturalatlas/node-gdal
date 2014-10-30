#include "typed_array.hpp"

#include <sstream>

namespace node_gdal {

//https://github.com/joyent/node/issues/4201#issuecomment-9837340

Handle<Value> TypedArray::New(GDALDataType type, unsigned int length)  {
	NanEscapableScope();

	Handle<Value> val;
	Handle<Function> constructor;
	Local<Object> global = NanGetCurrentContext()->Global();

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
			NanThrowError("Unsupported array type"); 
			return NanEscapeScope(NanUndefined());
	}


	// make ArrayBuffer
	val = global->Get(NanNew("ArrayBuffer"));

	if(val.IsEmpty() || !val->IsFunction()) {
		NanThrowError("Error getting ArrayBuffer constructor");
		return NanEscapeScope(NanUndefined());
	}

	constructor = val.As<Function>();
	Local<Value> size = NanNew<Integer>(length * GDALGetDataTypeSize(type) / 8);
	Local<Value> array_buffer = constructor->NewInstance(1, &size);

	if(array_buffer.IsEmpty() || !array_buffer->IsObject()) {
		NanThrowError("Error allocating ArrayBuffer");
		return NanEscapeScope(NanUndefined());
	}


	// make TypedArray
	val = global->Get(NanNew(name));

	if(val.IsEmpty() || !val->IsFunction()) {
		NanThrowError("Error getting typed array constructor");
		return NanEscapeScope(NanUndefined());
	}

	constructor = val.As<Function>();
	Local<Object> array = constructor->NewInstance(1, &array_buffer);

	if(array.IsEmpty() || !array->IsObject()) {
		NanThrowError("Error creating TypedArray");
		return NanEscapeScope(NanUndefined());
	}

	return NanEscapeScope(array);
}

GDALDataType TypedArray::Identify(Handle<Object> obj) {	
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

void* TypedArray::Validate(Handle<Object> obj, GDALDataType type, int min_length){
	//validate array
	NanEscapableScope();

	if(!obj->HasIndexedPropertiesInExternalArrayData()) {
		NanThrowError("Object has no external array data");
		return NULL;
	}
	GDALDataType src_type = TypedArray::Identify(obj);
	if(type == GDT_Unknown) {
		NanThrowTypeError("Unable to identify GDAL datatype of passed array object");
		return NULL;
	}
	if(src_type != type) {
		std::ostringstream ss;
		ss << "Array type does not match band data type (" 
		   << "array: " << GDALGetDataTypeName(src_type)
		   << " band: " << GDALGetDataTypeName(type) << ")";

		NanThrowTypeError(ss.str().c_str());
		return NULL;
	}
	if(TypedArray::Length(obj) < min_length) {
		std::ostringstream ss;
		ss << "Array length must be greater than or equal to " << min_length; 

		NanThrowError(ss.str().c_str());
		return NULL;
	}
	return TypedArray::Data(obj);
}

int TypedArray::Length(Handle<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayDataLength();
}
void* TypedArray::Data(Handle<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayData();
}

} //node_gdal namespace