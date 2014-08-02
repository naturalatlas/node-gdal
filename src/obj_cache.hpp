
#ifndef __OBJ_CACHE_H__
#define __OBJ_CACHE_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

#include <map>

template <typename K>
struct ObjectCacheItem {
	v8::Persistent<v8::Object> handle;
	K key;
	K alias;
};

// a class for maintaining a map of native pointers and persistent JS handles
// objects are removed from cache and their destructor is called when they go out of scope

template <typename K>
class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(K key, v8::Handle<v8::Object> obj);
	void add(K key, K alias, v8::Handle<v8::Object> obj);

	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists
	v8::Handle<v8::Object> get(K key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(K key);
	void erase(K key);

	ObjectCache();
	~ObjectCache();

private:
	ObjectCacheItem<K> getItem(K key);
	void erase(ObjectCacheItem<K> key);
	static void WeakCallback(v8::Persistent<v8::Value> object, void *parameter);
	std::map<K, ObjectCacheItem<K> > cache;
	std::map<K, K> aliases;
};

template <typename K>
struct CallbackParameters {
	ObjectCacheItem<K> item;
	ObjectCache<K> *cache;
};

template <typename K>
ObjectCache<K>::ObjectCache()
	: cache(), aliases()
{
}

template <typename K>
ObjectCache<K>::~ObjectCache()
{
}

template <typename K>
void ObjectCache<K>::add(K key, K alias, v8::Handle<v8::Object> obj)
{
	ObjectCacheItem<K> item;
	item.key    = key;
	item.alias  = alias;
	item.handle = v8::Persistent<v8::Object>::New(obj);

	CallbackParameters<K> *params = (CallbackParameters<K>*) malloc(sizeof(CallbackParameters<K>)) ;
	params->item  = item;
	params->cache = this;

	item.handle.MakeWeak(params, ObjectCache::WeakCallback);

	//add it to the map
	cache[key] = item;
	if(alias) {
		aliases[alias] = key;
	}
}


template <typename K>
void ObjectCache<K>::add(K key, v8::Handle<v8::Object> obj)
{
	add(key, NULL, obj);
}

template <typename K>
bool ObjectCache<K>::has(K key)
{
	return cache.count(key) > 0 || aliases.count(key) > 0;
}

template <typename K>
v8::Handle<v8::Object> ObjectCache<K>::get(K key)
{
	return getItem(key).handle;
}

template <typename K>
ObjectCacheItem<K> ObjectCache<K>::getItem(K key)
{
	//return handle to existing object if already wrapped
	//check by calling has() first
	if (aliases.count(key) > 0) {
		key = aliases[key];
	}
	return cache[key];
}


template <typename K>
void ObjectCache<K>::erase(K key)
{
	if(has(key)){
		erase(getItem(key));
	}
}

template <typename K>
void ObjectCache<K>::erase(ObjectCacheItem<K> item)
{

	cache.erase(item.key);
	if(item.alias){
		aliases.erase(item.alias);
	}
}

template <typename K>
void ObjectCache<K>::WeakCallback(v8::Persistent<v8::Value> object, void *parameter)
{
	//called when only reference to object is weak - after garbage collection
	CallbackParameters<K> *params = (CallbackParameters<K>*) parameter;

	//remove it from the map
	params->cache->erase(params->item);

	//clear the reference to it
	object.Dispose();
	object.Clear();

	free(params);
}

#endif