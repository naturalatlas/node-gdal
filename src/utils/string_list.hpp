
#ifndef __STRING_LIST_H__
#define __STRING_LIST_H__

// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

using namespace v8;

namespace node_gdal {

// A class for parsing a V8::Value and constructing a GDAL string list
//
// inputs:
// {key: value, ...}, ["key=value", ...]
//
// outputs:
// ["key=value", ...]

class StringList {
public:
	int parse(Local<Value> value);

	StringList();
	~StringList();

	inline char **get() {
		return list;
	}
private:
	char** list;
	std::string* strlist;
};

}

#endif