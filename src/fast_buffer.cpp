//from: http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/

#include <cstring>
#include "fast_buffer.hpp"

Handle<Value> FastBuffer::New(unsigned char *data, int length) 
{
	HandleScope scope;

	node::Buffer *slowBuffer = node::Buffer::New(length);

	memcpy(node::Buffer::Data(slowBuffer), data, length);

	v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();
	v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));
	v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer->handle_, v8::Integer::New(length), v8::Integer::New(0) };
	
	v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

	return scope.Close(actualBuffer);
}