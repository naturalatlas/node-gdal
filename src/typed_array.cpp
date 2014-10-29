#include "gdal_common.hpp"
#include "typed_array.hpp"

namespace node_gdal {

//https://github.com/joyent/node/issues/4201#issuecomment-9837340

Handle<Value> TypedArray::New(GDALDataType type, unsigned int length)  {
	NanEscapableScope();

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

	Local<Object> global = NanGetCurrentContext()->Global();
	Handle<Value> val = global->Get(NanNew(name));

	if(val.IsEmpty() || !val->IsFunction()) {
		NanThrowError("Error getting typed array constructor");
		return NanEscapeScope(NanUndefined());
	}

	Handle<Function> constructor = val.As<Function>();

	Local<Value>  size  = Integer::NewFromUnsigned(length);
	Local<Object> array = constructor->NewInstance(1, &size);

	if(array.IsEmpty() || !array->IsObject()) {
		NanThrowError("Error allocating array");
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
int TypedArray::Length(Handle<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayDataLength();
}
void* TypedArray::Data(Handle<Object> obj) {
	return obj->GetIndexedPropertiesExternalArrayData();
}

} //node_gdal namespace