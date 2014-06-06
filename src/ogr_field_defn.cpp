
#include "ogr_common.hpp"
#include "ogr_field_defn.hpp"

using namespace node_ogr;

Persistent<FunctionTemplate> FieldDefn::constructor;

void FieldDefn::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FieldDefn::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("FieldDefn"));

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setName", setName);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getType", getType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setType", setType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getJustify", getJustify);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setJustify", setJustify);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getWidth", getWidth);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setWidth", setWidth);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getPrecision", getPrecision);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setPrecision", setPrecision);
  NODE_SET_PROTOTYPE_METHOD(constructor, "isIgnored", isIgnored);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setIgnored", setIgnored);

  target->Set(String::NewSymbol("FieldDefn"), constructor->GetFunction());
}

FieldDefn::FieldDefn(OGRFieldDefn *layer)
: ObjectWrap(),
  this_(layer),
  owned_(false)
{}

FieldDefn::FieldDefn()
: ObjectWrap(),
  this_(0),
  owned_(false)
{
}

FieldDefn::~FieldDefn()
{
  if (owned_ && this_) {
    delete this_;
  }
  this_ = NULL;
}

Handle<Value> FieldDefn::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void* ptr = ext->Value();
    FieldDefn *f = static_cast<FieldDefn *>(ptr);
    f->Wrap(args.This());
    return args.This();
  } else {
    std::string field_name("");
    OGRFieldType field_type(OFTString);

    NODE_ARG_STR(0, "field name", field_name);
    NODE_ARG_ENUM(1, "field type", OGRFieldType, field_type);

    FieldDefn* def = new FieldDefn(new OGRFieldDefn(field_name.c_str(), field_type));
    def->owned_ = true;
    def->Wrap(args.This());
  }

  return args.This();
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def) {
  HandleScope scope;
  return scope.Close(FieldDefn::New(def, false));
}

Handle<Value> FieldDefn::New(OGRFieldDefn *def, bool owned) {
  HandleScope scope;

  if(!def) return Null();

  //make a copy of fielddefn owned by a featuredefn
  // + no need to track when a featuredefn is destroyed
  // + no need to throw errors when a method trys to modify an owned read-only fielddefn
  // - is slower
  
  if(!owned) def = new OGRFieldDefn(def);

  FieldDefn *wrapped = new FieldDefn(def);
  wrapped->owned_ = true;
  
  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = FieldDefn::constructor->GetFunction()->NewInstance(1, &ext);

  return scope.Close(obj);
}

Handle<Value> FieldDefn::toString(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(String::New("FieldDefn"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, getName, SafeString, GetNameRef);
NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, getType, Integer, GetType);
NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, isIgnored, Boolean, IsIgnored);
NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, getJustify, Integer, GetJustify);
NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, getWidth, Integer, GetWidth);
NODE_WRAPPED_METHOD_WITH_RESULT(FieldDefn, getPrecision, Integer, GetPrecision);
NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(FieldDefn, setJustify, SetJustify, OGRJustification, "justification");
NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(FieldDefn, setType, SetType, OGRFieldType, "field type");
NODE_WRAPPED_METHOD_WITH_1_INTEGER_PARAM(FieldDefn, setPrecision, SetPrecision, "field precision");
NODE_WRAPPED_METHOD_WITH_1_INTEGER_PARAM(FieldDefn, setWidth, SetWidth, "field width");
NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(FieldDefn, setIgnored, SetIgnored, "is ignored");
NODE_WRAPPED_METHOD_WITH_1_STRING_PARAM(FieldDefn, setName, SetName, "field name");
