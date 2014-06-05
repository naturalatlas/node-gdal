
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

template <typename K>
class ObjectCache {
public:
	//map a native pointer to a handle to the V8 obj that wraps it
	void add(K key, v8::Handle<v8::Object> obj);
	
	//add an alias for a native pointer 
	//- useful for mapping a clone and an original to the same V8 obj
	void addAlias(K key, K alias);
	
	//fetch the V8 obj that wraps the native pointer (or alias)
	//call has() before calling to get() to ensure handle exists 
	v8::Handle<v8::Object> get(K key);

	//check if native pointer has been wrapped, or if an alias wraps it
	bool has(K key);
	
	//check if native pointer has been added as an alias
	bool hasAlias(K alias);

	ObjectCache();
	~ObjectCache();

private:
	static void WeakCallback(v8::Persistent<v8::Value> object, void *parameter);
	void erase(K key);
	std::map<K, v8::Persistent<v8::Object> > cache;
	std::map<K, K> aliases;
};

template <typename K>
struct CallbackParameters
{
  K key;
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
void ObjectCache<K>::add(K key, v8::Handle<v8::Object> obj) {
  //create persistent handle
  v8::Persistent<v8::Object> ptr = v8::Persistent<v8::Object>::New(obj);

  CallbackParameters<K>* params = (CallbackParameters<K>*) malloc(sizeof(CallbackParameters<K>)) ;
  params->key = key; 
  params->cache = this;

  ptr.MakeWeak(params, ObjectCache::WeakCallback);

  //add it to the map
  cache[key] = ptr;
}

template <typename K>
void ObjectCache<K>::addAlias(K key, K alias){
  aliases[alias] = key;
}

template <typename K>
bool ObjectCache<K>::has(K key) {
  return cache.count(key) > 0 || hasAlias(key);
}

template <typename K>
bool ObjectCache<K>::hasAlias(K alias) {
  return aliases.count(alias) > 0;
}

template <typename K>
v8::Handle<v8::Object> ObjectCache<K>::get(K key){
  //return handle to existing object if already wrapped
  //check by calling has() first
  if (hasAlias(key)) key = aliases[key];
  return cache[key];
}

template <typename K>
void ObjectCache<K>::erase(K key){
  cache.erase(key);
}

template <typename K>
void ObjectCache<K>::WeakCallback(v8::Persistent<v8::Value> object, void *parameter) {
  //called when only reference to object is weak - after garbage collection
  CallbackParameters<K> *params = (CallbackParameters<K>*) parameter;

  //remove it from the map
  params->cache->erase(params->key);

  //clear the reference to it
  object.Dispose();
  object.Clear();

  free(params);
}

#endif