
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

//TODO: This could use some serious cleaning

#define ERASED_FROM_CACHE 1
#define WEAK_CALLBACK_CALLED 2

template <typename K>
struct ObjectCacheItem {
	Nan::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> obj;
	K *key;
	K *alias;
	char status;
	void *cache;
	int uid; // a uniq id so that the weak callback can distinguish if the object in the cache with the given key is referring to the same object as when the persistent pointer was made
};

// a class for maintaining a map of native pointers and persistent JS handles
// objects are removed from cache and their destructor is called when they go out of scope

template <typename K, typename W>
class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(K *key, v8::Local<v8::Object> obj);
	void add(K *key, K *alias, v8::Local<v8::Object> obj);

	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists
	v8::Local<v8::Object> get(K *key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(K *key);
	void erase(K *key);

	ObjectCache();
	~ObjectCache();

private:
	static void cacheWeakCallback(const Nan::WeakCallbackInfo<ObjectCacheItem<K>> &data);
	ObjectCacheItem<K>* getItem(K *key);
	void erase(ObjectCacheItem<K> *key);
	std::map<K*, ObjectCacheItem<K>* > cache;
	std::map<K*, K*> aliases;
	int uid;
};

template <typename K, typename W>
ObjectCache<K, W>::ObjectCache()
	: cache(), aliases(), uid(0)
{
}

template <typename K, typename W>
ObjectCache<K, W>::~ObjectCache()
{
}

template <typename K, typename W>
void ObjectCache<K, W>::cacheWeakCallback(const Nan::WeakCallbackInfo<ObjectCacheItem<K>> &data)
{
	//called when only reference to object is weak - after object destructor is called (... or before, who knows)
	ObjectCacheItem<K> *item = (ObjectCacheItem<K>*) data.GetParameter();
	ObjectCache<K, W> *cache = (ObjectCache<K, W>*) item->cache;
	K *key = (K*) item->key;

	LOG("ObjectCache Weak Callback [%p]", key);
	
	// dispose of the wrapped object before the handle in the cache gets deleted by NAN
	// but do not dispose if destructor has already been called
	if(cache->has(key)) {
		LOG("Key still in ObjectCache [%p]", key);
		//double check that the item in the cache isnt something new (meaning object was already disposed)
		if(cache->getItem(key)->uid == item->uid) { 
			LOG("And it points to object that generated the weak callback [%p]", key);
			W* wrapped = Nan::ObjectWrap::Unwrap<W>(Nan::New(item->obj));
			wrapped->dispose();
		}
	}

	item->status |= WEAK_CALLBACK_CALLED;

	if(item->status == (WEAK_CALLBACK_CALLED | ERASED_FROM_CACHE)){
		delete item;
		LOG("Deleted Object Cache Item [%p]", key);
	}
}

template <typename K, typename W>
void ObjectCache<K, W>::add(K *key, K *alias, v8::Local<v8::Object> obj)
{
	ObjectCacheItem<K> *item = new ObjectCacheItem<K>();
	item->key    = key;
	item->alias  = alias;
	item->cache  = this;
	item->uid    = uid++;
	item->status = 0;
	item->obj    = Nan::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>>(obj);

	item->obj.SetWeak(item, cacheWeakCallback, Nan::WeakCallbackType::kParameter);

	//add it to the map
	cache[key] = item;
	if(alias) {
		aliases[alias] = key;
	}
}

template <typename K, typename W>
void ObjectCache<K, W>::add(K *key, v8::Local<v8::Object> obj)
{
	add(key, NULL, obj);
}

template <typename K, typename W>
bool ObjectCache<K, W>::has(K *key)
{
	return cache.count(key) > 0 || aliases.count(key) > 0;
}

template <typename K, typename W>
v8::Local<v8::Object> ObjectCache<K, W>::get(K *key)
{	
	return Nan::New(getItem(key)->obj);
}

template <typename K, typename W>
ObjectCacheItem<K>* ObjectCache<K, W>::getItem(K *key)
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
void ObjectCache<K, W>::erase(ObjectCacheItem<K> *item)
{
	K* key = (K*) item->key;
	cache.erase(key);
	if(item->alias){
		aliases.erase(item->alias);
	}
	item->status |= ERASED_FROM_CACHE;

	if(item->status == (WEAK_CALLBACK_CALLED | ERASED_FROM_CACHE)){
		delete item;
		LOG("Deleted Object Cache Item [%p]", key);
	}
}

#endif