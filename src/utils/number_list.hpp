#ifndef __NUMBER_LIST_H__
#define __NUMBER_LIST_H__

// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

using namespace v8;

namespace node_gdal {

// Classes for parsing a V8::Value and constructing a list of numbers. Destroys the list when the list goes out of scope

class IntegerList {
public:
	int parse(Local<Value> value);

	IntegerList();
	IntegerList(const char* name);
	~IntegerList();

	inline int* get() {
		return list;
	}
	inline int length() {
		return len;
	}
private:
	int* list;
	unsigned int len;
	const char* name;
};

class DoubleList {
public:
	int parse(Local<Value> value);

	DoubleList();
	DoubleList(const char* name);
	~DoubleList();

	inline double* get() {
		return list;
	}
	inline int length() {
		return len;
	}
private:
	double* list;
	unsigned int len;
	const char* name;
};

}

#endif
