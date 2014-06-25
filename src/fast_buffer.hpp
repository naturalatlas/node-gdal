#ifndef __NODE_GDAL_FAST_BUFFER_H__
#define __NODE_GDAL_FAST_BUFFER_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

class FastBuffer {
public:
	static v8::Handle<v8::Value> New(unsigned char * data, int length);
};

#endif
