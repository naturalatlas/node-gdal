
#ifndef __GDAL_COMMON_H__
#define __GDAL_COMMON_H__

#include <gdal_version.h>
#include <cpl_error.h>
#include <stdio.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

namespace node_gdal {
	extern FILE *log_file;
}

#ifdef ENABLE_LOGGING
#define LOG(fmt, ...) if (node_gdal::log_file) { fprintf(node_gdal::log_file, fmt"\n", __VA_ARGS__); fflush(node_gdal::log_file); }
#else
#define LOG(fmt, ...)
#endif

//NanNew(null) -> seg fault
class SafeString {
public:
	static v8::Handle<v8::Value> New(const char * data) {
		NanEscapableScope();
    if (!data) {
			return NanEscapeScope(NanNull());
		} else {
			return NanEscapeScope(NanNew<v8::String>(data));
		}
	}
};

inline const char* getOGRErrMsg(int err)
{
  if(err == 6) {
    //get more descriptive error
    //TODO: test if all OGRErr failures report an error msg
    return CPLGetLastErrorMsg();
  }
  switch(err) {
  case 0:
    return "No error";
  case 1:
    return "Not enough data";
  case 2:
    return "Not enough memory";
  case 3:
    return "Unsupported geometry type";
  case 4:
    return "Unsupported operation";
  case 5:
    return "Corrupt Data";
  case 6:
    return "Failure";
  case 7:
    return "Unsupported SRS";
  default:
    return "Invalid Error";
  }
};

#define NODE_THROW_CPLERR(err) NanThrowError(CPLGetLastErrorMsg());

#define NODE_THROW_LAST_CPLERR NODE_THROW_CPLERR

#define NODE_THROW_OGRERR(err) NanThrowError(getOGRErrMsg(err));

#define ATTR(t, name, get, set)                                         \
    t->InstanceTemplate()->SetAccessor(NanNew(name), get, set);

#define ATTR_DONT_ENUM(t, name, get, set)                                         \
    t->InstanceTemplate()->SetAccessor(NanNew(name), get, set, Handle<Value>(), DEFAULT, DontEnum);

NAN_SETTER(READ_ONLY_SETTER);

#define IS_WRAPPED(obj, type) NanHasInstance(type::constructor, obj)

// ----- object property conversion -------

#define NODE_DOUBLE_FROM_OBJ(obj, key, var)                                                               \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (!obj->HasOwnProperty(sym)){                                                                         \
     NanThrowError("Object must contain property \"" key "\""); NanReturnUndefined();                     \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsNumber()){                                                                                  \
    NanThrowTypeError("Property \"" key "\" must be a number");                                           \
    NanReturnUndefined();                                                                                 \
  }                                                                                                       \
  var = val->NumberValue();                                                                               \
}

#define NODE_INT_FROM_OBJ(obj, key, var)                                                                  \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (!obj->HasOwnProperty(sym)){                                                                         \
     NanThrowError("Object must contain property \"" key "\""); NanReturnUndefined();                     \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsNumber()){                                                                                  \
    NanThrowTypeError("Property \"" key "\" must be a number");                                           \
    NanReturnUndefined();                                                                                 \
  }                                                                                                       \
  var = val->Int32Value();                                                                                \
}

#define NODE_STR_FROM_OBJ(obj, key, var)                                                                  \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (!obj->HasOwnProperty(sym)){                                                                         \
     NanThrowError("Object must contain property \"" key "\""); NanReturnUndefined();                     \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsString()){                                                                                  \
      NanThrowTypeError("Property \"" key "\" must be a string");                                         \
      NanReturnUndefined();                                                                               \
  }                                                                                                       \
  var = *NanUtf8String(val);                                                                              \
}

#define NODE_WRAPPED_FROM_OBJ(obj, key, type, var)                                                        \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (!obj->HasOwnProperty(sym)){                                                                         \
     NanThrowError("Object must contain property \"" key "\""); NanReturnUndefined();                     \
  }                                                                                                       \
  Local<Value> val = obj->Get(sym);                                                                       \
  if (!val->IsObject() || val->IsNull() || !NanHasInstance(type::constructor, val)) {                     \
      NanThrowTypeError("Property \"" key "\" must be a " #type " object");                               \
      NanReturnUndefined();                                                                               \
  }                                                                                                       \
  var = ObjectWrap::Unwrap<type>(val.As<Object>());                                                       \
  if(!var->get()) {                                                                                       \
      NanThrowError(key ": " #type " object has already been destroyed");                                 \
      NanReturnUndefined();                                                                               \
  }                                                                                                       \
}

#define NODE_WRAPPED_FROM_OBJ_OPT(obj, key, type, var)                                                    \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (obj->HasOwnProperty(sym)){                                                                          \
    Local<Value> val = obj->Get(sym);                                                                     \
    if(val->IsObject() && NanHasInstance(type::constructor, val)){                                        \
      var = ObjectWrap::Unwrap<type>(val.As<Object>());                                                   \
      if(!var->get()) {                                                                                   \
          NanThrowError(key ": " #type " object has already been destroyed");                             \
          NanReturnUndefined();                                                                           \
      }                                                                                                   \
    } else if(!val->IsNull() && !val->IsUndefined()){                                                     \
      NanThrowTypeError(key "property must be a " #type " object");                                       \
      NanReturnUndefined();                                                                               \
    }                                                                                                     \
  }                                                                                                       \
}

#define NODE_DOUBLE_FROM_OBJ_OPT(obj, key, var)                                                           \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (obj->HasOwnProperty(sym)){                                                                          \
    Local<Value> val = obj->Get(sym);                                                                     \
    if (!val->IsNumber()){                                                                                \
      NanThrowTypeError("Property \"" key "\" must be a number");                                         \
      NanReturnUndefined();                                                                               \
    }                                                                                                     \
    var = val->NumberValue();                                                                             \
  }                                                                                                       \
}

#define NODE_INT_FROM_OBJ_OPT(obj, key, var)                                                              \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (obj->HasOwnProperty(sym)){                                                                          \
    Local<Value> val = obj->Get(sym);                                                                     \
    if (!val->IsNumber()){                                                                                \
      NanThrowTypeError("Property \"" key "\" must be a number");                                         \
      NanReturnUndefined();                                                                               \
    }                                                                                                     \
    var = val->Int32Value();                                                                              \
  }                                                                                                       \
}

#define NODE_STR_FROM_OBJ_OPT(obj, key, var)                                                              \
{                                                                                                         \
  Local<String> sym = NanNew(key);                                                                        \
  if (obj->HasOwnProperty(sym)){                                                                          \
    Local<Value> val = obj->Get(sym);                                                                     \
    if (!val->IsString()){                                                                                \
      NanThrowTypeError("Property \"" key "\" must be a string");                                         \
      NanReturnUndefined();                                                                               \
    }                                                                                                     \
    var = *NanUtf8String(val);                                                                            \
  }                                                                                                       \
}

// ----- argument conversion -------

//determine field index based on string/numeric js argument
//f -> OGRFeature* or OGRFeatureDefn*

#define ARG_FIELD_ID(num, f, var) {                                    \
  if (args[num]->IsString()) {                                         \
    std::string field_name = *NanUtf8String(args[num]);                \
    var = f->GetFieldIndex(field_name.c_str());                        \
    if (field_index == -1) {                                           \
      NanThrowError("Specified field name does not exist");            \
      NanReturnUndefined();                                            \
    }                                                                  \
  } else if (args[num]->IsInt32()) {                                   \
    var = args[num]->Int32Value();                                     \
    if (var < 0 || var >= f->GetFieldCount()) {                        \
      NanThrowRangeError("Invalid field index");                       \
      NanReturnUndefined();                                            \
    }                                                                  \
  } else {                                                             \
    NanThrowTypeError("Field index must be integer or string");        \
    NanReturnUndefined();                                              \
  }                                                                    \
}

#define NODE_ARG_INT(num, name, var)                                                                           \
  if (args.Length() < num + 1) {                                                                               \
    NanThrowError(name " must be given"); NanReturnUndefined();                                                \
  }                                                                                                            \
  if (!args[num]->IsNumber()) {                                                                                \
    NanThrowTypeError(name " must be an integer"); NanReturnUndefined();                                       \
  }                                                                                                            \
  var = static_cast<int>(args[num]->IntegerValue());


#define NODE_ARG_ENUM(num, name, enum_type, var)                                                                                       \
  if (args.Length() < num + 1) {                                                                                                       \
    NanThrowError(name " must be given"); NanReturnUndefined();                                                                        \
  }                                                                                                                                    \
  if (!args[num]->IsInt32() && !args[num]->IsUint32()) {                                                                               \
    NanThrowTypeError(name " must be of type " #enum_type); NanReturnUndefined();                                                      \
  }                                                                                                                                    \
  var = enum_type(args[num]->Uint32Value());


#define NODE_ARG_BOOL(num, name, var)                                                                          \
  if (args.Length() < num + 1) {                                                                               \
    NanThrowError(name " must be given"); NanReturnUndefined();                                                \
  }                                                                                                            \
  if (!args[num]->IsBoolean()) {                                                                               \
    NanThrowTypeError(name " must be an boolean"); NanReturnUndefined();                                       \
  }                                                                                                            \
  var = static_cast<bool>(args[num]->BooleanValue());


#define NODE_ARG_DOUBLE(num, name, var)                                                                      \
  if (args.Length() < num + 1) {                                                                             \
    NanThrowError(name " must be given"); NanReturnUndefined();                                              \
  }                                                                                                          \
  if (!args[num]->IsNumber()) {                                                                              \
    NanThrowTypeError(name " must be a number"); NanReturnUndefined();                                       \
  }                                                                                                          \
  var = static_cast<double>(args[num]->NumberValue());


#define NODE_ARG_ARRAY(num, name, var)                                                                       \
  if (args.Length() < num + 1) {                                                                             \
    NanThrowError(name " must be given"); NanReturnUndefined();                                              \
  }                                                                                                          \
  if (!args[num]->IsArray()) {                                                                               \
    return NanThrowTypeError((std::string(name) + " must be an array").c_str());                             \
  }                                                                                                          \
  var = args[num].As<Array>();


#define NODE_ARG_OBJECT(num, name, var)                                                                       \
  if (args.Length() < num + 1) {                                                                              \
    NanThrowError(name " must be given"); NanReturnUndefined();                                               \
  }                                                                                                           \
  if (!args[num]->IsObject()) {                                                                               \
    return NanThrowTypeError((std::string(name) + " must be an object").c_str());                             \
  }                                                                                                           \
  var = args[num].As<Object>();


#define NODE_ARG_WRAPPED(num, name, type, var)                                                                                           \
  if (args.Length() < num + 1) {                                                                                                         \
    NanThrowError(name " must be given"); NanReturnUndefined();                                                                          \
  }                                                                                                                                      \
  if (args[num]->IsNull() || args[num]->IsUndefined() || !NanNew(type::constructor)->HasInstance(args[num])) {                           \
    NanThrowTypeError(name " must be an instance of " #type); NanReturnUndefined();                                                      \
  }                                                                                                                                      \
  var = ObjectWrap::Unwrap<type>(args[num].As<Object>());                                                                                \
  if (!var->get()) {                                                                                                                     \
    NanThrowError(#type" parameter already destroyed"); NanReturnUndefined();                                                            \
  }

#define NODE_ARG_STR(num, name, var)                                                                          \
  if (args.Length() < num + 1) {                                                                              \
    NanThrowError(name " must be given"); NanReturnUndefined();                                               \
  }                                                                                                           \
  if (!args[num]->IsString()) {                                                                               \
    NanThrowTypeError(name " must be an string"); NanReturnUndefined();                                       \
  }                                                                                                           \
  var = (*NanUtf8String(args[num]))

// ----- optional argument conversion -------

#define NODE_ARG_INT_OPT(num, name, var)                                                                         \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsInt32()) {                                                                                  \
      var = static_cast<int>(args[num]->IntegerValue());                                                         \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                               \
      NanThrowTypeError(name " must be an integer"); NanReturnUndefined();                                       \
    }                                                                                                            \
  }


#define NODE_ARG_ENUM_OPT(num, name, enum_type, var)                                                             \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsInt32() || args[num]->IsUint32()) {                                                         \
      var = static_cast<enum_type>(args[num]->Uint32Value());                                                    \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                               \
      NanThrowTypeError(name " must be an integer"); NanReturnUndefined();                                       \
    }                                                                                                            \
  }


#define NODE_ARG_BOOL_OPT(num, name, var)                                                                        \
  if (args.Length() > num) {                                                                                     \
    if (args[num]->IsBoolean()) {                                                                                \
      var = static_cast<bool>(args[num]->BooleanValue());                                                        \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                               \
      NanThrowTypeError(name " must be an boolean"); NanReturnUndefined();                                       \
    }                                                                                                            \
  }


#define NODE_ARG_OPT_STR(num, name, var)                                                                        \
  if (args.Length() > num) {                                                                                    \
    if (args[num]->IsString()) {                                                                                \
      var = *NanUtf8String(args[num]);                                                                          \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                              \
      NanThrowTypeError(name " must be an string"); NanReturnUndefined();                                       \
    }                                                                                                           \
  }


#define NODE_ARG_DOUBLE_OPT(num, name, var)                                                                    \
  if (args.Length() > num) {                                                                                   \
    if (args[num]->IsNumber()) {                                                                               \
      var = static_cast<double>(args[num]->NumberValue());                                                     \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                             \
      NanThrowTypeError(name " must be a number"); NanReturnUndefined();                                       \
    }                                                                                                          \
  }


#define NODE_ARG_WRAPPED_OPT(num, name, type, var)                                                                                         \
  if (args.Length() > num && !args[num]->IsNull() && !args[num]->IsUndefined()) {                                                          \
    if (!NanNew(type::constructor)->HasInstance(args[num])) {                                                                              \
      NanThrowTypeError(name " must be an instance of " #type); NanReturnUndefined();                                                      \
    }                                                                                                                                      \
    var = ObjectWrap::Unwrap<type>(args[num].As<Object>());                                                                                \
    if (!var->get()) {                                                                                                                     \
      NanThrowError(#type" parameter already destroyed");                                                                                  \
      NanReturnUndefined();                                                                                                                \
    }                                                                                                                                      \
  }


#define NODE_ARG_ARRAY_OPT(num, name, var)                                                                     \
  if (args.Length() > num) {                                                                                   \
    if (args[num]->IsArray()) {                                                                                \
      var = args[num].As<Array>();                                                                             \
    } else if(!args[num]->IsNull() && !args[num]->IsUndefined()) {                                             \
      NanThrowTypeError(name " must be an array"); NanReturnUndefined();                                       \
    }                                                                                                          \
  }

// ----- wrapped methods w/ results-------

#define NODE_WRAPPED_METHOD_WITH_RESULT(klass, method, result_type, wrapped_method)                               \
NAN_METHOD(klass::method)                                                                                         \
{                                                                                                                 \
  NanScope();                                                                                                     \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                            \
  if (!obj->this_) {                                                                                              \
    NanThrowError(#klass" object has already been destroyed");                                                    \
    NanReturnUndefined();                                                                                         \
  }                                                                                                               \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method()));                                              \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(klass, method, result_type, wrapped_method, param_type, param_name) \
NAN_METHOD(klass::method)                                                                                                   \
{                                                                                                                           \
  NanScope();                                                                                                               \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                      \
  if (!obj->this_)   return NanThrowError(#klass" object has already been destroyed");                                      \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method(param->get())));                                            \
}

#define NODE_WRAPPED_METHOD_WITH_RESULT_1_ENUM_PARAM(klass, method, result_type, wrapped_method, enum_type, param_name) \
NAN_METHOD(klass::method)                                                                                               \
{                                                                                                                       \
  NanScope();                                                                                                           \
  enum_type param;                                                                                                      \
  NODE_ARG_ENUM(0, #param_name, enum_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                  \
  if (!obj->this_) {                                                                                                    \
    NanThrowError(#klass" object has already been destroyed");                                                          \
    NanReturnUndefined();                                                                                               \
  }                                                                                                                     \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method(param)));                                               \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(klass, method, result_type, wrapped_method, param_name)        \
NAN_METHOD(klass::method)                                                                                             \
{                                                                                                                     \
  NanScope();                                                                                                         \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                \
  if (!obj->this_) {                                                                                                  \
    NanThrowError(#klass" object has already been destroyed");                                                        \
    NanReturnUndefined();                                                                                             \
  }                                                                                                                   \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method(param.c_str())));                                     \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_INTEGER_PARAM(klass, method, result_type, wrapped_method, param_name) \
NAN_METHOD(klass::method)                                                                                       \
{                                                                                                               \
  NanScope();                                                                                                   \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) {                                                                                            \
    NanThrowError(#klass" object has already been destroyed");                                                  \
    NanReturnUndefined();                                                                                       \
  }                                                                                                             \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method(param)));                                       \
}


#define NODE_WRAPPED_METHOD_WITH_RESULT_1_DOUBLE_PARAM(klass, method, result_type, wrapped_method, param_name) \
NAN_METHOD(klass::method)                                                                                      \
{                                                                                                              \
  NanScope();                                                                                                  \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) {                                                                                           \
    NanThrowError(#klass" object has already been destroyed");                                                 \
    NanReturnUndefined();                                                                                      \
  }                                                                                                            \
  NanReturnValue(NanNew<result_type>(obj->this_->wrapped_method(param)));                                      \
}

// ----- wrapped methods w/ CPLErr result (throws) -------

#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT(klass, method, wrapped_method)                                       \
NAN_METHOD(klass::method)                                                                                           \
{                                                                                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                              \
  if (!obj->this_) {                                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                                      \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  int err = obj->this_->wrapped_method();                                                                           \
  if(err) {                                                                                                         \
    NODE_THROW_CPLERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name)       \
NAN_METHOD(klass::method)                                                                                                   \
{                                                                                                                           \
  NanScope();                                                                                                               \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                      \
  if (!obj->this_) {                                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                                      \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  int err = obj->this_->wrapped_method(param->get());                                                               \
  if(err) {                                                                                                         \
    NODE_THROW_CPLERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_STRING_PARAM(klass, method, wrapped_method, param_name)              \
NAN_METHOD(klass::method)                                                                                             \
{                                                                                                                     \
  NanScope();                                                                                                         \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                \
  if (!obj->this_) {                                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                                      \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  int err = obj->this_->wrapped_method(param.c_str());                                                              \
  if(err) {                                                                                                         \
    NODE_THROW_CPLERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_INTEGER_PARAM(klass, method, wrapped_method, param_name)       \
NAN_METHOD(klass::method)                                                                                       \
{                                                                                                               \
  NanScope();                                                                                                   \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) {                                                                                            \
    NanThrowError(#klass" object has already been destroyed");                                                  \
    NanReturnUndefined();                                                                                       \
  }                                                                                                             \
  int err = obj->this_->wrapped_method(param);                                                                  \
  if (err) {                                                                                                    \
    NODE_THROW_CPLERR(err);                                                                                     \
    NanReturnUndefined();                                                                                       \
  }                                                                                                             \
  NanReturnUndefined();                                                                                         \
}


#define NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)       \
NAN_METHOD(klass::method)                                                                                      \
{                                                                                                              \
  NanScope();                                                                                                  \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) {                                                                                           \
    NanThrowError(#klass" object has already been destroyed");                                                 \
    NanReturnUndefined();                                                                                      \
  }                                                                                                            \
  int err =obj->this_->wrapped_method(param);                                                                       \
  if(err) {                                                                                                         \
    NODE_THROW_CPLERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}

// ----- wrapped methods w/ OGRErr result (throws) -------

#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(klass, method, wrapped_method)                                       \
NAN_METHOD(klass::method)                                                                                           \
{                                                                                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                              \
  if (!obj->this_) {                                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                                      \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  int err = obj->this_->wrapped_method();                                                                           \
  if (err) {                                                                                                        \
    NODE_THROW_OGRERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name)       \
NAN_METHOD(klass::method)                                                                                                   \
{                                                                                                                           \
  NanScope();                                                                                                               \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                      \
  if (!obj->this_) {                                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                                      \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  int err = obj->this_->wrapped_method(param->get());                                                               \
  if (err) {                                                                                                        \
    NODE_THROW_OGRERR(err);                                                                                         \
    NanReturnUndefined();                                                                                           \
  }                                                                                                                 \
  NanReturnUndefined();                                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(klass, method, wrapped_method, param_name)              \
NAN_METHOD(klass::method)                                                                                             \
{                                                                                                                     \
  NanScope();                                                                                                         \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                \
  if (!obj->this_) {                                                                                                  \
    NanThrowError(#klass" object has already been destroyed");                                                        \
    NanReturnUndefined();                                                                                             \
  }                                                                                                                   \
  int err = obj->this_->wrapped_method(param.c_str());                                                                \
  if (err) {                                                                                                          \
    NODE_THROW_OGRERR(err);                                                                                           \
    NanReturnUndefined();                                                                                             \
  }                                                                                                                   \
  NanReturnUndefined();                                                                                               \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(klass, method, wrapped_method, param_name)       \
NAN_METHOD(klass::method)                                                                                       \
{                                                                                                               \
  NanScope();                                                                                                   \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) {                                                                                            \
    NanThrowError(#klass" object has already been destroyed");                                                  \
    NanReturnUndefined();                                                                                       \
  }                                                                                                             \
  int err = obj->this_->wrapped_method(param);                                                                  \
  if (err) {                                                                                                    \
    NODE_THROW_OGRERR(err);                                                                                     \
    NanReturnUndefined();                                                                                       \
  }                                                                                                             \
  NanReturnUndefined();                                                                                         \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)       \
NAN_METHOD(klass::method)                                                                                      \
{                                                                                                              \
  NanScope();                                                                                                  \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) {                                                                                           \
    NanThrowError(#klass" object has already been destroyed");                                                 \
    NanReturnUndefined();                                                                                      \
  }                                                                                                            \
  int err =obj->this_->wrapped_method(param);                                                                  \
  if (err) {                                                                                                   \
    NODE_THROW_OGRERR(err);                                                                                    \
    NanReturnUndefined();                                                                                      \
  }                                                                                                            \
  NanReturnUndefined();                                                                                        \
}

// ----- wrapped methods -------

#define NODE_WRAPPED_METHOD(klass, method, wrapped_method)           \
NAN_METHOD(klass::method)                                            \
{                                                                    \
  NanScope();                                                        \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());               \
  if (!obj->this_) {                                                 \
    NanThrowError(#klass" object has already been destroyed");       \
    NanReturnUndefined();                                            \
  }                                                                  \
  obj->this_->wrapped_method();                                      \
  NanReturnUndefined();                                              \
}


#define NODE_WRAPPED_METHOD_WITH_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name) \
NAN_METHOD(klass::method)                                                                               \
{                                                                                                       \
  NanScope();                                                                                           \
  param_type *param;                                                                                    \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                  \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                  \
  if (!obj->this_) {                                                                                    \
    NanThrowError(#klass" object has already been destroyed");                                          \
    NanReturnUndefined();                                                                               \
  }                                                                                                     \
  obj->this_->wrapped_method(param->get());                                                             \
  NanReturnUndefined();                                                                                 \
}


#define NODE_WRAPPED_METHOD_WITH_1_INTEGER_PARAM(klass, method, wrapped_method, param_name) \
NAN_METHOD(klass::method)                                                                   \
{                                                                                           \
  NanScope();                                                                               \
  int param;                                                                                \
  NODE_ARG_INT(0, #param_name, param);                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) {                                                                        \
    NanThrowError(#klass" object has already been destroyed");                              \
    NanReturnUndefined();                                                                   \
  }                                                                                         \
  obj->this_->wrapped_method(param);                                                        \
  NanReturnUndefined();                                                                     \
}


#define NODE_WRAPPED_METHOD_WITH_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)  \
NAN_METHOD(klass::method)                                                                   \
{                                                                                           \
  NanScope();                                                                               \
  double param;                                                                             \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) {                                                                        \
    NanThrowError(#klass" object has already been destroyed");                              \
    NanReturnUndefined();                                                                   \
  }                                                                                         \
  obj->this_->wrapped_method(param);                                                        \
  NanReturnUndefined();                                                                     \
}


#define NODE_WRAPPED_METHOD_WITH_1_BOOLEAN_PARAM(klass, method, wrapped_method, param_name) \
NAN_METHOD(klass::method)                                                                   \
{                                                                                           \
  NanScope();                                                                               \
  bool param;                                                                               \
  NODE_ARG_BOOL(0, #param_name, param);                                                     \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) {                                                                        \
    NanThrowError(#klass" object has already been destroyed");                              \
    NanReturnUndefined();                                                                   \
  }                                                                                         \
  obj->this_->wrapped_method(param);                                                        \
  NanReturnUndefined();                                                                     \
}


#define NODE_WRAPPED_METHOD_WITH_1_ENUM_PARAM(klass, method, wrapped_method, enum_type, param_name) \
NAN_METHOD(klass::method)                                                                           \
{                                                                                                   \
  NanScope();                                                                                       \
  enum_type param;                                                                                  \
  NODE_ARG_ENUM(0, #param_name, enum_type, param);                                                  \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                              \
  if (!obj->this_) {                                                                                \
    NanThrowError(#klass" object has already been destroyed");                                      \
    NanReturnUndefined();                                                                           \
  }                                                                                                 \
  obj->this_->wrapped_method(param);                                                                \
  NanReturnUndefined();                                                                             \
}


#define NODE_WRAPPED_METHOD_WITH_1_STRING_PARAM(klass, method, wrapped_method, param_name)  \
NAN_METHOD(klass::method)                                                                   \
{                                                                                           \
  NanScope();                                                                               \
  std::string param;                                                                        \
  NODE_ARG_STR(0, #param_name, param);                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                      \
  if (!obj->this_) {                                                                        \
    NanThrowError(#klass" object has already been destroyed");                              \
    NanReturnUndefined();                                                                   \
  }                                                                                         \
  obj->this_->wrapped_method(param.c_str());                                                \
  NanReturnUndefined();                                                                     \
}

#endif