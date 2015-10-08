#ifndef __NODE_TYPEDARRAY_H__
#define __NODE_TYPEDARRAY_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

namespace node_gdal {

// V8 Typed Arrays
//
// Int8Array    1  8-bit twos complement  signed integer  signed char
// Uint8Array   1  8-bit unsigned integer  unsigned  char
// Int16Array   2  16-bit twos complement  signed integer  short
// Uint16Array  2  16-bit unsigned integer  unsigned short
// Int32Array   4  32-bit twos complement  signed integer  int
// Uint32Array  4  32-bit unsigned integer  unsigned int
// Float32Array 4  32-bit IEEE floating point number  float
// Float64Array 8  64-bit IEEE floating point number  double

// GDALDataType {
//   GDT_Unknown = 0,
//   GDT_Byte = 1,
//   GDT_UInt16 = 2,
//   GDT_Int16 = 3,
//   GDT_UInt32 = 4,
//   GDT_Int32 = 5,
//   GDT_Float32 = 6,
//   GDT_Float64 = 7,
//   GDT_CInt16 = 8,
//   GDT_CInt32 = 9,
//   GDT_CFloat32 = 10,
//   GDT_CFloat64 = 11,
//   GDT_TypeCount = 12
// }

namespace TypedArray {

	Local<Value> New(GDALDataType type, unsigned int length);
	GDALDataType Identify(Local<Object> array);
	void* Validate(Local<Object> obj, GDALDataType type, int min_length);
	bool ValidateLength(int length, int min_length);
}

}
#endif
