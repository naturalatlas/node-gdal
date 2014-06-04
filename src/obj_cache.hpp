
#ifndef __OBJ_CACHE_H__
#define __OBJ_CACHE_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

#include <map>

// a class for maintaining a map of native pointers and persistent JS handles
// objects are removed from cache and their destructor is called when they go out of scope

class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(void *raw, v8::Handle<v8::Object> obj);
	
	//add an alias for a native pointer 
	//- useful for mapping a clone and an original to the same V8 obj
	void addAlias(void *raw, void *alias);
	
	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists 
	v8::Handle<v8::Object> get(void* raw);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(void *raw);
	
	//check if native pointer has been added as an alias
	bool hasAlias(void *alias);

	ObjectCache();
	~ObjectCache();

private:
	static void WeakCallback(v8::Persistent<v8::Value> object, void *parameter);
	void erase(void *raw);
	std::map<void*, v8::Persistent<v8::Object> > cache;
	std::map<void*, void* > aliases;
};

#endif