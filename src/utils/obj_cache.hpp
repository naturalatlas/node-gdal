
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

enum ObjectCacheItemStatus {
	ITEM_ACTIVE = 0,
	ITEM_RELEASED_FROM_CACHE = 1,
	ITEM_RELEASED_FROM_WEAK_CALLBACK = 2,
	ITEM_RELEASED_FROM_ALL = 3
};

template <typename K>
struct ObjectCacheItem {
	Nan::Persistent<v8::Object> obj;
	K *key;
	K *alias;
	ObjectCacheItemStatus status;
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
	W* getWrapped(K *key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(K *key);
	void erase(K *key);

	ObjectCache();
	~ObjectCache();

private:
	static void release(ObjectCacheItem<K> *item, ObjectCacheItemStatus status_change);
	static void cacheWeakCallback(const Nan::WeakCallbackInfo< ObjectCacheItem<K> > &data);
	ObjectCacheItem<K>* getItem(K *key);
	void erase(ObjectCacheItem<K> *key);
	std::map<K*, ObjectCacheItem<K>* > cache;
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
void ObjectCache<K, W>::cacheWeakCallback(const Nan::WeakCallbackInfo< ObjectCacheItem<K> > &data)
{
	ObjectCacheItem<K>* item = (ObjectCacheItem<K>*) data.GetParameter();
	LOG("ObjectCache Weak Callback [%p]", item->key);
	release(item, ITEM_RELEASED_FROM_WEAK_CALLBACK);
}


template <typename K, typename W>
void ObjectCache<K, W>::release(ObjectCacheItem<K> *item, ObjectCacheItemStatus status_change)
{
	item->status = (ObjectCacheItemStatus) (item->status | status_change);
	if(item->status == ITEM_RELEASED_FROM_ALL){
		delete item;
	}
}

template <typename K, typename W>
void ObjectCache<K, W>::add(K *key, K *alias, v8::Local<v8::Object> obj)
{
	ObjectCacheItem<K> *item = new ObjectCacheItem<K>();
	item->key    = key;
	item->alias  = alias;
	item->status = ITEM_ACTIVE;
	item->obj.Reset(obj);
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
bool ObjectCache<K, W>::has(K *key){
	return cache.count(key) > 0 || aliases.count(key) > 0;
};

template <typename K, typename W>
v8::Local<v8::Object> ObjectCache<K, W>::get(K *key)
{	
	Nan::EscapableHandleScope scope;
	return scope.Escape(Nan::New(getItem(key)->obj));
}

template <typename K, typename W>
W* ObjectCache<K, W>::getWrapped(K *key)
{	

	ObjectCacheItem<K> *item = getItem(key);
	v8::Local<v8::Object> obj = Nan::New(item->obj);

	bool is_near_death = item->obj.IsNearDeath();
	if(is_near_death){
		LOG("About to Unwrap Near Death handle [%p]", key);
	}

	W *unwrapped = Nan::ObjectWrap::Unwrap<W>(obj);

	if(is_near_death){
		LOG("Unwrapped Near Death handle [%p]", key);
	}
	return unwrapped;
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

	LOG("Erased Object Cache Item [%p]", key);
	release(item, ITEM_RELEASED_FROM_CACHE);
}

#endif