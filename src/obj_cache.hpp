
#ifndef __OBJ_CACHE_H__
#define __OBJ_CACHE_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

#include <map>

#include "gdal_common.hpp"

NAN_WEAK_CALLBACK(cacheWeakCallback)
{
	//called when only reference to object is weak - after garbage collection

	LOG("ObjectCache Weak Callback [%p]", data.GetParameter());
}

template <typename P>
struct ObjectCacheItem {
	v8::Persistent<v8::Object> handle;
	P key;
	P alias;
};

// a class for maintaining a map of native pointers and persistent JS handles
// objects are removed from cache and their destructor is called when they go out of scope

template <typename P>
class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(P key, v8::Handle<v8::Object> obj);
	void add(P key, P alias, v8::Handle<v8::Object> obj);

	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists
	v8::Handle<v8::Object> get(P key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(P key);
	void erase(P key);

	ObjectCache();
	~ObjectCache();

private:
	ObjectCacheItem<P> getItem(P key);
	void erase(ObjectCacheItem<P> key);
	std::map<P, ObjectCacheItem<P> > cache;
	std::map<P, P> aliases;
};

template <typename P>
ObjectCache<P>::ObjectCache()
	: cache(), aliases()
{
}

template <typename P>
ObjectCache<P>::~ObjectCache()
{
}

template <typename P>
void ObjectCache<P>::add(P key, P alias, v8::Handle<v8::Object> obj)
{
	ObjectCacheItem<P> item;
	item.key    = key;
	item.alias  = alias;
	item.handle = NanMakeWeakPersistent(obj, key, cacheWeakCallback)->persistent;

	//add it to the map
	cache[key] = item;
	if(alias) {
		aliases[alias] = key;
	}
}


template <typename P>
void ObjectCache<P>::add(P key, v8::Handle<v8::Object> obj)
{
	add(key, NULL, obj);
}

template <typename P>
bool ObjectCache<P>::has(P key)
{
	return cache.count(key) > 0 || aliases.count(key) > 0;
}

template <typename P>
v8::Handle<v8::Object> ObjectCache<P>::get(P key)
{
	return getItem(key).handle;
}

template <typename P>
ObjectCacheItem<P> ObjectCache<P>::getItem(P key)
{
	//return handle to existing object if already wrapped
	//check by calling has() first
	if (aliases.count(key) > 0) {
		key = aliases[key];
	}
	return cache[key];
}


template <typename P>
void ObjectCache<P>::erase(P key)
{
	if(has(key)){
		erase(getItem(key));
	}
}

template <typename P>
void ObjectCache<P>::erase(ObjectCacheItem<P> item)
{

	LOG("ObjectCache erasing [%p]", item.key);
	cache.erase(item.key);
	if(item.alias){
		LOG("ObjectCache erasing alias [%p]", item.alias);
		aliases.erase(item.alias);
	}
}

#endif