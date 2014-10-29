// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

#include <string>

#include "gdal_common.hpp"

using namespace v8;

NAN_SETTER(READ_ONLY_SETTER)
{
	NanScope();
	std::string name = *NanUtf8String(property);
	std::string err = name + " is a read-only property";
	NanThrowError(err.c_str());
}