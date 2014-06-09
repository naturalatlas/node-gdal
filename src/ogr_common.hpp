
#ifndef __OGR_COMMON_H__
#define __OGR_COMMON_H__

#include <v8.h>

#include "gdal_common.hpp"

inline const char* getOGRErrMsg(int err)
{
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

#define NODE_THROW_OGRERR(err) ThrowException(Exception::Error(String::New(getOGRErrMsg(err))));

// ----- wrapped methods w/ OGRErr result (throws) -------

#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(klass, method, wrapped_method)                                       \
Handle<Value> klass::method(const Arguments& args)                                                                  \
{                                                                                                                   \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                              \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                   \
  int err = obj->this_->wrapped_method();                                                                           \
  if (err) return NODE_THROW_OGRERR(err);                                                                           \
  return Undefined();                                                                                               \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(klass, method, wrapped_method, param_type, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                                          \
{                                                                                                                           \
  HandleScope scope;                                                                                                        \
  param_type *param;                                                                                                        \
  NODE_ARG_WRAPPED(0, #param_name, param_type, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                      \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                           \
  int err = obj->this_->wrapped_method(param->get());                                                                       \
  if (err) return NODE_THROW_OGRERR(err);                                                                                   \
  return Undefined();                                                                                                       \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(klass, method, wrapped_method, param_name)              \
Handle<Value> klass::method(const Arguments& args)                                                                    \
{                                                                                                                     \
  HandleScope scope;                                                                                                  \
  std::string param;                                                                                                  \
  NODE_ARG_STR(0, #param_name, param);                                                                                \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                                \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                                     \
  int err = obj->this_->wrapped_method(param.c_str());                                                                \
  if (err) return NODE_THROW_OGRERR(err);                                                                             \
  return Undefined();                                                                                                 \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(klass, method, wrapped_method, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                              \
{                                                                                                               \
  HandleScope scope;                                                                                            \
  int param;                                                                                                    \
  NODE_ARG_INT(0, #param_name, param);                                                                          \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                          \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                               \
  int err = obj->this_->wrapped_method(param);                                                                  \
  if (err) return NODE_THROW_OGRERR(err);                                                                       \
  return Undefined();                                                                                           \
}


#define NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_DOUBLE_PARAM(klass, method, wrapped_method, param_name)       \
Handle<Value> klass::method(const Arguments& args)                                                             \
{                                                                                                              \
  HandleScope scope;                                                                                           \
  double param;                                                                                                \
  NODE_ARG_DOUBLE(0, #param_name, param);                                                                      \
  klass *obj = ObjectWrap::Unwrap<klass>(args.This());                                                         \
  if (!obj->this_) return NODE_THROW(#klass" object has already been destroyed");                              \
  int err =obj->this_->wrapped_method(param);                                                                  \
  if (err) return NODE_THROW_OGRERR(err);                                                                      \
  return Undefined();                                                                                          \
}

#endif