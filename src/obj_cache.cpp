#include <stdlib.h>

#include "obj_cache.hpp"

struct CallbackParameters
{
  void *raw;
  ObjectCache *cache;
};

ObjectCache::ObjectCache()
: cache(),
aliases()
{
}

ObjectCache::~ObjectCache()
{
}

void ObjectCache::add(void *raw, v8::Handle<v8::Object> obj) {
  //create persistent handle
  v8::Persistent<v8::Object> ptr = v8::Persistent<v8::Object>::New(obj);

  CallbackParameters* params = (CallbackParameters*) malloc(sizeof(CallbackParameters)) ;
  params->raw = raw; 
  params->cache = this;

  ptr.MakeWeak(params, ObjectCache::WeakCallback);

  //add it to the map
  cache[raw] = ptr;
}
void ObjectCache::addAlias(void* raw, void* alias){
  aliases[alias] = raw;
}

bool ObjectCache::has(void *raw) {
  return cache.count(raw) > 0 || hasAlias(raw);
}
bool ObjectCache::hasAlias(void *alias) {
  return aliases.count(alias) > 0;
}

v8::Handle<v8::Object> ObjectCache::get(void* raw){
  //return handle to existing object if already wrapped
  //check by calling has() first
  if (hasAlias(raw)) raw = aliases[raw];
  return cache[raw];
}

void ObjectCache::erase(void *raw){
  cache.erase(raw);
}

void ObjectCache::WeakCallback(v8::Persistent<v8::Value> object, void *parameter) {
  //called when only reference to object is weak - after garbage collection
  CallbackParameters *params = (CallbackParameters*) parameter;

  //remove it from the map
  params->cache->erase(params->raw);

  //clear the reference to it
  object.Dispose();
  object.Clear();

  free(params);
}
