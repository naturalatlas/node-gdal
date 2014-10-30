
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

#include "../gdal_common.hpp"

template <typename K>
struct ObjectCacheItem {
	_NanWeakCallbackInfo<v8::Object, K> *cbinfo;
	K *key;
	K *alias;
};

// a class for maintaining a map of native pointers and persistent JS handles
// objects are removed from cache and their destructor is called when they go out of scope

template <typename K, typename W>
class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(K *key, v8::Handle<v8::Object> obj);
	void add(K *key, K *alias, v8::Handle<v8::Object> obj);

	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists
	v8::Handle<v8::Object> get(K *key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(K *key);
	void erase(K *key);

	ObjectCache();
	~ObjectCache();

private:
	NAN_WEAK_CALLBACK(cacheWeakCallback);

	ObjectCacheItem<K> getItem(K *key);
	void erase(ObjectCacheItem<K> key);
	std::map<K*, ObjectCacheItem<K> > cache;
	std::map<K*, K*> aliases;
};

template <typename K, typename W>
ObjectCache<K, W>::ObjectCache()
	: cache(), aliases()
{
}

template <typename K, typename W>
ObjectCache<K, W>::~ObjectCache()
{
}


template <typename K, typename W>
template<typename T, typename P>                                           \
void ObjectCache<K, W>::cacheWeakCallback(const _NanWeakCallbackData<T, P> &data)
{
	//called when only reference to object is weak - after object destructor is called (... or before, who knows)

	LOG("ObjectCache Weak Callback [%p]", data.GetParameter());

	//dispose of the wrapped object before the handle in the cache gets deleted by NAN
	W* wrapped = node::ObjectWrap::Unwrap<W>(data.GetValue());
	wrapped->dispose();
}

template <typename K, typename W>
void ObjectCache<K, W>::add(K *key, K *alias, v8::Handle<v8::Object> obj)
{
	ObjectCacheItem<K> item;
	item.key    = key;
	item.alias  = alias;
	item.cbinfo = NanMakeWeakPersistent(obj, key, cacheWeakCallback);

	//add it to the map
	cache[key] = item;
	if(alias) {
		aliases[alias] = key;
	}
}


template <typename K, typename W>
void ObjectCache<K, W>::add(K *key, v8::Handle<v8::Object> obj)
{
	add(key, NULL, obj);
}

template <typename K, typename W>
bool ObjectCache<K, W>::has(K *key)
{
	return cache.count(key) > 0 || aliases.count(key) > 0;
}

template <typename K, typename W>
v8::Handle<v8::Object> ObjectCache<K, W>::get(K *key)
{
	return NanNew(getItem(key).cbinfo->persistent);
}

template <typename K, typename W>
ObjectCacheItem<K> ObjectCache<K, W>::getItem(K *key)
{
	//return handle to existing object if already wrapped
	//check by calling has() first
	if (aliases.count(key) > 0) {
		key = aliases[key];
	}
	return cache[key];
}


template <typename K, typename W>
void ObjectCache<K, W>::erase(K *key)
{
	if(has(key)){
		erase(getItem(key));
	}
}

template <typename K, typename W>
void ObjectCache<K, W>::erase(ObjectCacheItem<K> item)
{

	LOG("ObjectCache erasing [%p]", item.key);
	cache.erase(item.key);
	if(item.alias){
		LOG("ObjectCache erasing alias [%p]", item.alias);
		aliases.erase(item.alias);
	}
}

#endif