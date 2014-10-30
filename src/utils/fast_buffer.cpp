//from: http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/

#include <cstring>
#include "fast_buffer.hpp"

Handle<Value> FastBuffer::New(unsigned char *data, int length) 
{
	NanEscapableScope();

	Local<Object> slowBuffer = NanNewBufferHandle(length);

	memcpy(node::Buffer::Data(slowBuffer), data, length);

	Local<Object> globalObj = NanGetCurrentContext()->Global();
	Local<Function> bufferConstructor = globalObj->Get(NanNew("Buffer")).As<Function>();
	Handle<Value> constructorArgs[3] = { slowBuffer, NanNew<Integer>(length), NanNew<Integer>(0) };
	
	Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

	return NanEscapeScope(actualBuffer);
}