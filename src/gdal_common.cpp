// v8
#include <v8.h>

// node
#include <node.h>

#include <string>

#include "gdal_common.hpp"

using namespace v8;

void READ_ONLY_SETTER(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	std::string name = TOSTR(property);
	std::string err = name + " is a read-only property";
	NODE_THROW(err.c_str());
}