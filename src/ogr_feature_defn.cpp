
#include "ogr_common.hpp"
#include "ogr_feature_defn.hpp"
#include "ogr_field_defn.hpp"

using namespace node_ogr;

Persistent<FunctionTemplate> FeatureDefn::constructor;

void FeatureDefn::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureDefn::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("FeatureDefn"));

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldCount", getFieldCount);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldDefn", getFieldDefn);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addFieldDefn", addFieldDefn);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldIndex", getFieldIndex);
  NODE_SET_PROTOTYPE_METHOD(constructor, "deleteFieldDefn", deleteFieldDefn);
  NODE_SET_PROTOTYPE_METHOD(constructor, "reorderFieldDefns", reorderFieldDefns);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getGeomType", getGeomType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setGeomType", setGeomType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isGeometryIgnored", isGeometryIgnored);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometryIgnored", setGeometryIgnored);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isStyleIgnored", isStyleIgnored);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setStyleIgnored", setStyleIgnored);
  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);

  target->Set(String::NewSymbol("FeatureDefn"), constructor->GetFunction());
}

FeatureDefn::FeatureDefn(OGRFeatureDefn *layer)
: ObjectWrap(),
  this_(layer),
  owned_(true)
{}

FeatureDefn::FeatureDefn()
: ObjectWrap(),
  this_(0),
  owned_(true)
{
}

FeatureDefn::~FeatureDefn()
{
  if (owned_ && this_) {
    this_->Release();
  }
  this_ = NULL;
}

Handle<Value> FeatureDefn::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.IsConstructCall())
      return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
      Local<External> ext = Local<External>::Cast(args[0]);
      void* ptr = ext->Value();
      FeatureDefn *f = static_cast<FeatureDefn *>(ptr);
      f->Wrap(args.This());
      return args.This();
  }

  return args.This();
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def) {
  HandleScope scope;
  return scope.Close(FeatureDefn::New(def, false));
}

Handle<Value> FeatureDefn::New(OGRFeatureDefn *def, bool owned) {
  HandleScope scope;
  
  if(!def) return Null();
  
  //make a copy of featuredefn owned by a layer
  // + no need to track when a layer is destroyed
  // + no need to throw errors when a method trys to modify an owned read-only featuredefn
  // - is slower
  
  if(!owned) def = def->Clone();

  FeatureDefn *wrapped = new FeatureDefn(def);
  wrapped->owned_ = true;

  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = FeatureDefn::constructor->GetFunction()->NewInstance(1, &ext);

  return scope.Close(obj);
}

Handle<Value> FeatureDefn::toString(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(String::New("FeatureDefn"));
}


NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, getName, SafeString, GetName);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, getFieldCount, Integer, GetFieldCount);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, getGeomType, Integer, GetGeomType);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, clone, FeatureDefn, Clone);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, isGeometryIgnored, Boolean, IsGeometryIgnored);
NODE_WRAPPED_METHOD_WITH_RESULT(FeatureDefn, isStyleIgnored, Boolean, IsStyleIgnored);
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(FeatureDefn, getFieldIndex, Integer, GetFieldIndex, "field name");
NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(FeatureDefn, setGeomType, SetGeomType, OGRwkbGeometryType, "geometry type");
NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(FeatureDefn, setGeometryIgnored, SetGeometryIgnored, "ignore");
NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(FeatureDefn, setStyleIgnored, SetStyleIgnored, "ignore");
NODE_WRAPPED_METHOD_WITH_1_WRAPPED_PARAM(FeatureDefn, addFieldDefn, AddFieldDefn, FieldDefn, "feature definition");


Handle<Value> FeatureDefn::reorderFieldDefns(const Arguments& args)
{
  HandleScope scope;
  Handle<Array> field_map = Array::New(0);

  NODE_ARG_ARRAY(0, "field map", field_map);

  int *field_map_array = NULL;
  OGRErr err = 0;

  if (field_map->Length() > 0) {
    field_map_array = new int[field_map->Length()];
  }

  FeatureDefn *defn = ObjectWrap::Unwrap<FeatureDefn>(args.This());

  if (field_map_array) {
    err = defn->this_->ReorderFieldDefns(field_map_array);

    delete [] field_map_array;
  }

  if(err) return NODE_THROW_OGRERR(err);
  return Undefined();
}

Handle<Value> FeatureDefn::deleteFieldDefn(const Arguments& args)
{
  HandleScope scope;
  int field_index;
  NODE_ARG_INT(0, "field index", field_index);

  FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
  if(!def->this_) return NODE_THROW("Feature object already destroyed");

  if(field_index < 0 || field_index >= def->this_->GetFieldCount())
    return NODE_THROW("Invalid field index");

  OGRErr err = def->this_->DeleteFieldDefn(field_index);
  if(err) return NODE_THROW_OGRERR(err);
  return Undefined();
}

Handle<Value> FeatureDefn::getFieldDefn(const Arguments& args)
{
  HandleScope scope;
  int field_index;
  NODE_ARG_INT(0, "field index", field_index);

  FeatureDefn *def = ObjectWrap::Unwrap<FeatureDefn>(args.This());
  if(!def->this_) return NODE_THROW("Feature object already destroyed");

  if(field_index < 0 || field_index >= def->this_->GetFieldCount())
    return NODE_THROW("Invalid field index");

  return scope.Close(FieldDefn::New(def->this_->GetFieldDefn(field_index)));
}