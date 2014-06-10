
#ifndef __GDAL_COMMON_H__
#define __GDAL_COMMON_H__

#include <v8.h>

//String::New(null) -> seg fault
class SafeString {
public:
	static v8::Handle<v8::Value> New(const char * data) {
		if (!data) {
			return v8::Null();
		} else {
			return v8::String::New(data);
		}
	}
};

#define TOSTR(obj) (*String::Utf8Value((obj)->ToString()))

#define NODE_THROW(msg) ThrowException(Exception::Error(String::New(msg)));

#define NODE_THROW_CPLERR(err) ThrowException(Exception::Error(String::New(CPLGetLastErrorMsg())));

#define ATTR(t, name, get, set)                                         \
    t->InstanceTemplate()->SetAccessor(String::NewSymbol(name), get, set);

void READ_ONLY_SETTER(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo &info);

template <typename T, typename K>
class ClosedPtr {
public:
	static v8::Handle<v8::Value> Closed(K *raw) {
		if (!raw) {
			return v8::Null();
		}
		v8::HandleScope scope;
		T *wrapped = new T(raw);
		v8::Handle<v8::Value> ext = v8::External::New(wrapped);
		v8::Handle<v8::Object> obj = T::constructor->GetFunction()->NewInstance(1, &ext);
		return scope.Close(obj);
	}
};

// ----- object property conversion -------

#define NODE_DOUBLE_FROM_OBJ(obj, key, var)                                                               \
{                                                                                                         \
  Local<String> sym = String::NewSymbol(key);                                                             \
  if (!obj->Has(sym)){                                                                                    \
     return ThrowException(Exception::Error(String::New("Object must contain property \"" key "\"")));    \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsNumber()){                                                                                  \
    return ThrowException(Exception::Error(String::New("Property \"" key "\" must be a number")));        \
  }                                                                                                       \
  var = val->NumberValue();                                                                               \
}

#define NODE_STR_FROM_OBJ(obj, key, var)                                                                  \
{                                                                                                         \
  Local<String> sym = String::NewSymbol(key);                                                             \
  if (!obj->Has(sym)){                                                                                    \
     return ThrowException(Exception::Error(String::New("Object must contain property \"" key "\"")));    \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsString()){                                                                                  \
    return ThrowException(Exception::Error(String::New("Property \"" key "\" must be a string")));        \
  }                                                                                                       \
  var = (*String::Utf8Value(val->ToString()));                                                            \
}

#define NODE_DOUBLE_FROM_OBJ_OPT(obj, key, var)                                                           \
{                                                                                                         \
  Local<String> sym = String::NewSymbol(key);                                                             \
  if (obj->Has(sym)){                                                                                     \
    Local<Value> val = obj->Get(sym);                                                                     \
    if (!val->IsNumber()){                                                                                \
      return ThrowException(Exception::Error(String::New("Property \"" key "\" must be a number")));      \
    }                                                                                                     \
    var = val->NumberValue();                                                                             \
  }                                                                                                       \
}

#define NODE_STR_FROM_OBJ_OPT(obj, key, var)                                                              \
{                                                                                                         \
  Local<String> sym = String::NewSymbol(key);                                                             \
  if (obj->Has(sym)){                                                                                     \
    Local<Value> val = obj->Get(sym);                                                                     \
    if (!val->IsString()){                                                                                \
      return ThrowException(Exception::Error(String::New("Property \"" key "\" must be a string")));      \
    }                                                                                                     \
    var = (*String::Utf8Value(val->ToString()));                                                          \
  }                                                                                                       \
}

// ----- argument conversion -------

#define NODE_ARG_INT(num, name, var)                                                                           \
  if (args.Length() < num + 1) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));      \
  }                                                                                                            \
  if (!args[num]->IsNumber()) {                                                                                \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an integer").c_str()))); \
  }                                                                                                            \
  var = static_cast<int>(args[num]->IntegerValue());


#define NODE_ARG_ENUM(num, name, enum_type, var)                                                                                       \
  if (args.Length() < num + 1) {                                                                                                       \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));                              \
  }                                                                                                                                    \
  if (!args[num]->IsInt32()) {                                                                                                         \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be of type " + std::string(#enum_type)).c_str()))); \
  }                                                                                                                                    \
  var = enum_type(args[num]->IntegerValue());


#define NODE_ARG_BOOL(num, name, var)                                                                          \
  if (args.Length() < num + 1) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));      \
  }                                                                                                            \
  if (!args[num]->IsBoolean()) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an boolean").c_str()))); \
  }                                                                                                            \
  var = static_cast<bool>(args[num]->BooleanValue());


#define NODE_ARG_DOUBLE(num, name, var)                                                                      \
  if (args.Length() < num + 1) {                                                                             \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));    \
  }                                                                                                          \
  if (!args[num]->IsNumber()) {                                                                              \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be a number").c_str()))); \
  }                                                                                                          \
  var = static_cast<double>(args[num]->NumberValue());


#define NODE_ARG_ARRAY(num, name, var)                                                                       \
  if (args.Length() < num + 1) {                                                                             \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));    \
  }                                                                                                          \
  if (!args[num]->IsArray()) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an array").c_str()))); \
  }                                                                                                          \
  var = Handle<Array>::Cast(args[num]);


#define NODE_ARG_OBJECT(num, name, var)                                                                       \
  if (args.Length() < num + 1) {                                                                              \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));     \
  }                                                                                                           \
  if (!args[num]->IsObject()) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an object").c_str()))); \
  }                                                                                                           \
  var = Handle<Object>::Cast(args[num]);


#define NODE_ARG_WRAPPED(num, name, type, var)                                                                                           \
  if (args.Length() < num + 1) {                                                                                                         \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));                                \
  }                                                                                                                                      \
  Local<Object> var##_obj = args[num]->ToObject();                                                                                       \
  if (var##_obj->IsNull() || var##_obj->IsUndefined() || !type::constructor->HasInstance(var##_obj)) {                                   \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an instance of " + std::string(#name)).c_str()))); \
  }                                                                                                                                      \
  var = ObjectWrap::Unwrap<type>(var##_obj);                                                                                             \
  if (!var->get()) return ThrowException(Exception::Error(String::New(#type" parameter already destroyed")));


#define NODE_ARG_STR(num, name, var)                                                                          \
  if (args.Length() < num + 1) {                                                                              \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be given").c_str())));     \
  }                                                                                                           \
  if (!args[num]->IsString()) {                                                                               \
    return ThrowException(Exception::Error(String::New((std::string(name) + " must be an string").c_str()))); \
  }                                                                                                           \
  var = (*String::Utf8Value((args[num])->ToString()))

// ----- optional argument conversion -------

#define NODE_ARG_INT_OPT(num, name, var)                                                                         \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsInt32()) {                                                                                  \
      var = static_cast<int>(args[num]->IntegerValue());                                                         \
    } else {                                                                                                     \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an integer").c_str()))); \
    }                                                                                                            \
  }


#define NODE_ARG_ENUM_OPT(num, name, enum_type, var)                                                             \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsInt32()) {                                                                                  \
      var = static_cast<enum_type>(args[num]->IntegerValue());                                                   \
    } else {                                                                                                     \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an integer").c_str()))); \
    }                                                                                                            \
  }


#define NODE_ARG_BOOL_OPT(num, name, var)                                                                        \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsBoolean()) {                                                                                \
      var = static_cast<bool>(args[num]->BooleanValue());                                                        \
    } else {                                                                                                     \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an boolean").c_str()))); \
    }                                                                                                            \
  }


#define NODE_ARG_OPT_STR(num, name, var)                                                                        \
  if (args.Length() > num) {                                                                                    \
    if (args[num]->IsString()) {                                                                                \
      var = TOSTR(args[num]);                                                                                   \
    } else {                                                                                                    \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an string").c_str()))); \
    }                                                                                                           \
  }


#define NODE_ARG_DOUBLE_OPT(num, name, var)                                                                    \
  if (args.Length() > num) {                                                                                   \
    if (args[num]->IsNumber()) {                                                                               \
      var = static_cast<double>(args[num]->NumberValue());                                                     \
    } else {                                                                                                   \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be a number").c_str()))); \
    }                                                                                                          \
  }


#define NODE_ARG_WRAPPED_OPT(num, name, type, var)                                                                                         \
  if (args.Length() > num) {                                                                                                               \
    Local<Object> var##_obj = args[num]->ToObject();                                                                                       \
    if (var##_obj->IsNull() || var##_obj->IsUndefined() || !type::constructor->HasInstance(var##_obj)) {                                   \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an instance of " + std::string(#name)).c_str()))); \
    }                                                                                                                                      \
    var = ObjectWrap::Unwrap<type>(var##_obj);                                                                                             \
    if (!var->get()) return ThrowException(Exception::Error(String::New(#type" parameter already destroyed")));                            \
  }


#define NODE_ARG_ARRAY_OPT(num, name, var)                                                                     \
  if (args.Length() > num) {                                                                                   \
    if (args[num]->IsArray()) {                                                                                \
      var = Handle<Array>::Cast(args[num]);                                                                    \
    } else {                                                                                                   \
      return ThrowException(Exception::Error(String::New((std::string(name) + " must be an array").c_str()))); \
    }                                                                                                          \
  }

// ----- wrapped methods w/ results-------

#define NODE_WRAPPED_METHOD_WITH_RESULT(klass, method, result_type, wrapped_method)                               \
Handle<Value> klass::method(const Arguments& args)                                                                \
{                                                                                                                 \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                            \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed"); \
  return HandleScope().Close(result_type::New(obj->this_->wrapped_method()));     \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(klass, method, result_type, wrapped_method, param_type, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                                          \
{                                                                                                                           \
  HandleScope scope;                                                                                                        \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                             \
  if (!obj->this_)   return NODE_THROW(#klass" object has already been destroyed");                \
  return scope.Close(result_type::New(obj->this_->wrapped_method(param->get())));                  \
}

#define NODE_WRAPPED_METHOD_WITH_RESULT_1_ENUM_PARAM(klass, method, result_type, wrapped_method, enum_type, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                                      \
{                                                                                                                       \
  HandleScope scope;                                                                                                    \
  enum_type param;                                                                                                      \
  NODE_ARG_ENUM(0, #param_name, enum_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                             \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                  \
  return scope.Close(result_type::New(obj->this_->wrapped_method(param)));                         \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(klass, method, result_type, wrapped_method, param_name)        \
Handle<Value> klass::method(const Arguments& args)                                                                    \
{                                                                                                                     \
  HandleScope scope;                                                                                                  \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                             \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                  \
  return scope.Close(result_type::New(obj->this_->wrapped_method(param.c_str())));                 \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_INTEGER_PARAM(klass, method, result_type, wrapped_method, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                              \
{                                                                                                               \
  HandleScope scope;                                                                                            \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                               \
  return scope.Close(result_type::New(obj->this_->wrapped_method(param)));                                      \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_DOUBLE_PARAM(klass, method, result_type, wrapped_method, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                             \
{                                                                                                              \
  HandleScope scope;                                                                                           \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                              \
  return scope.Close(result_type::New(obj->this_->wrapped_method(param)));                                     \
}

// ----- wrapped methods w/ CPLErr result (throws) -------

#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT(klass, method, wrapped_method)                                       \
Handle<Value> klass::method(const Arguments& args)                                                                  \
{                                                                                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                              \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                   \
  int err = obj->this_->wrapped_method();                                                                           \
  if (err) return NODE_THROW_CPLERR(err);                                                                           \
  return Undefined();                                                                                               \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                                          \
{                                                                                                                           \
  HandleScope scope;                                                                                                        \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                           \
  int err = obj->this_->wrapped_method(param->get());                                                                       \
  if (err) return NODE_THROW_CPLERR(err);                                                                                   \
  return Undefined();                                                                                                       \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_STRING_PARAM(klass, method, wrapped_method, param_name)              \
Handle<Value> klass::method(const Arguments& args)                                                                    \
{                                                                                                                     \
  HandleScope scope;                                                                                                  \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                     \
  int err = obj->this_->wrapped_method(param.c_str());                                                                \
  if (err) return NODE_THROW_CPLERR(err);                                                                             \
  return Undefined();                                                                                                 \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_INTEGER_PARAM(klass, method, wrapped_method, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                              \
{                                                                                                               \
  HandleScope scope;                                                                                            \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                               \
  int err = obj->this_->wrapped_method(param);                                                                  \
  if (err) return NODE_THROW_CPLERR(err);                                                                       \
  return Undefined();                                                                                           \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                             \
{                                                                                                              \
  HandleScope scope;                                                                                           \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                              \
  int err =obj->this_->wrapped_method(param);                                                                  \
  if (err) return NODE_THROW_CPLERR(err);                                                                      \
  return Undefined();                                                                                          \
}

// ----- wrapped methods -------

#define NODE_WRAPPED_METHOD(klass, method, wrapped_method)           \
Handle<Value> klass::method(const Arguments& args)                   \
{                                                                    \
  HandleScope scope;                                                 \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());               \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed"); \
  obj->this_->wrapped_method();                                      \
  return Undefined();                                                \
}


#define NODE_WRAPPED_METHOD_WITH_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                      \
{                                                                                                       \
  HandleScope scope;                                                                                    \
  param_type *param;                                                                                    \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                  \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                  \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                       \
  obj->this_->wrapped_method(param->get());                                                             \
  return Undefined();                                                                                   \
}


#define NODE_WRAPPED_METHOD_WITH_1_INTEGER_PARAM(klass, method, wrapped_method, param_name) \
Handle<Value> klass::method(const Arguments& args)                                          \
{                                                                                           \
  HandleScope scope;                                                                        \
  int param;                                                                                \
  NODE_ARG_INT(0, #param_name, param);                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");            \
  obj->this_->wrapped_method(param);                                                        \
  return Undefined();                                                                       \
}


#define NODE_WRAPPED_METHOD_WITH_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)  \
Handle<Value> klass::method(const Arguments& args)                                          \
{                                                                                           \
  HandleScope scope;                                                                        \
  double param;                                                                             \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");           \
  obj->this_->wrapped_method(param);                                                        \
  return Undefined();                                                                       \
}


#define NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(klass, method, wrapped_method, param_name) \
Handle<Value> klass::method(const Arguments& args)                                          \
{                                                                                           \
  HandleScope scope;                                                                        \
  bool param;                                                                               \
  NODE_ARG_BOOL(0, #param_name, param);                                                     \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");           \
  obj->this_->wrapped_method(param);                                                        \
  return Undefined();                                                                       \
}


#define NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(klass, method, wrapped_method, enum_type, param_name) \
Handle<Value> klass::method(const Arguments& args)                                                  \
{                                                                                                   \
  HandleScope scope;                                                                                \
  enum_type param;                                                                                  \
  NODE_ARG_ENUM(0, #param_name, enum_type, param);                                                  \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                              \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                   \
  obj->this_->wrapped_method(param);                                                                \
  return Undefined();                                                                               \
}


#define NODE_WRAPPED_METHOD_WITH_1_STRING_PARAM(klass, method, wrapped_method, param_name)  \
Handle<Value> klass::method(const Arguments& args)                                          \
{                                                                                           \
  HandleScope scope;                                                                        \
  std::string param;                                                                        \
  NODE_ARG_STR(0, #param_name, param);                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");           \
  obj->this_->wrapped_method(param.c_str());                                                \
  return Undefined();                                                                       \
}

#endif