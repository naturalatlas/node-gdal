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
	Nan::HandleScope scope;
	std::string name = *Nan::Utf8String(property);
	std::string err = name + " is a read-only property";
	Nan::ThrowError(err.c_str());
}