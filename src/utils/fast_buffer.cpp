//from: http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/

#include <cstring>
#include "fast_buffer.hpp"

Local<Value> FastBuffer::New(unsigned char *data, int length) 
{
	Nan::EscapableHandleScope scope;

	Local<Object> slowBuffer = Nan::NewBuffer(length).ToLocalChecked();

	memcpy(node::Buffer::Data(slowBuffer), data, length);

	Local<Object> globalObj = Nan::GetCurrentContext()->Global();
	Local<Function> bufferConstructor = globalObj->Get(Nan::New("Buffer").ToLocalChecked()).As<Function>();
	Local<Value> constructorArgs[3] = { slowBuffer, Nan::New<Integer>(length), Nan::New<Integer>(0) };
	
	Local<Object> actualBuffer = Nan::NewInstance(bufferConstructor, 3, constructorArgs).ToLocalChecked();

	return scope.Escape(actualBuffer);
}
