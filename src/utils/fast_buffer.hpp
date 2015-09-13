#ifndef __NODE_GDAL_FAST_BUFFER_H__
#define __NODE_GDAL_FAST_BUFFER_H__

// node
#include <node.h>
#include <node_buffer.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

using namespace v8;
using namespace node;

class FastBuffer {
public:
	static v8::Local<v8::Value> New(unsigned char * data, int length);
};

#endif
