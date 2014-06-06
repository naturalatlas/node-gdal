
#include "ogr_common.hpp"
#include "ogr_layer.hpp"
#include "ogr_driver.hpp"
#include "ogr_datasource.hpp"

using namespace node_ogr;

Persistent<FunctionTemplate> Driver::constructor;
ObjectCache<OGRSFDriver*> Driver::cache;

void Driver::Initialize(Handle<Object> target) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Driver::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("Driver"));

  NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
  NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
  NODE_SET_PROTOTYPE_METHOD(constructor, "open", open);
  NODE_SET_PROTOTYPE_METHOD(constructor, "testCapability", testCapability);
  NODE_SET_PROTOTYPE_METHOD(constructor, "createDataSource", createDataSource);
  NODE_SET_PROTOTYPE_METHOD(constructor, "deleteDataSource", deleteDataSource);
  NODE_SET_PROTOTYPE_METHOD(constructor, "copyDataSource", copyDataSource);

  target->Set(String::NewSymbol("Driver"), constructor->GetFunction());
}

Driver::Driver(OGRSFDriver *driver)
: ObjectWrap(),
  this_(driver)
{}

Driver::Driver()
: ObjectWrap(),
  this_(0)
{
}

Driver::~Driver()
{
}

Handle<Value> Driver::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void *ptr = ext->Value();
    Driver *f =  static_cast<Driver *>(ptr);
    f->Wrap(args.This());
    return args.This();
  } else {
    return ThrowException(String::New("Cannot create driver directly."));
  }

  return args.This();
}

Handle<Value> Driver::New(OGRSFDriver *raw) {
  HandleScope scope;
  
  if(!raw) return v8::Null();
  if(cache.has(raw)) return cache.get(raw);

  Driver *wrapped = new Driver(raw);
  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = Driver::constructor->GetFunction()->NewInstance(1, &ext);
  
  cache.add(raw, obj);

  return scope.Close(obj);
}

Handle<Value> Driver::toString(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(String::New("Driver"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(Driver, getName, SafeString, GetName);
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(Driver, testCapability, Boolean, TestCapability, "capability");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(Driver, deleteDataSource, DeleteDataSource, "data source");

Handle<Value> Driver::open(const Arguments& args)
{
  HandleScope scope;

  std::string datasource_name;
  bool is_update = false;

  NODE_ARG_STR(0, "datasource name", datasource_name);
  NODE_ARG_BOOL_OPT(1, "update mode", is_update);

  Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
  OGRDataSource *ds = driver->this_->Open(datasource_name.c_str(), is_update);
  if(!ds) return NODE_THROW("Error opening datasource.");

  return scope.Close(Datasource::New(ds));
}


Handle<Value> Driver::createDataSource(const Arguments& args)
{
  HandleScope scope;

  std::string datasource_name;
  Handle<Array> options = Array::New(0);

  NODE_ARG_STR(0, "datasource name", datasource_name);
  NODE_ARG_ARRAY_OPT(1, "creation option", options);

  Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

  char **create_options = NULL;

  if (options->Length() > 0) {
    create_options = new char * [options->Length()];
    for (unsigned int i= 0; i < options->Length(); i++) {
      create_options[i] = TOSTR(options->Get(i));
    }
  }

  OGRDataSource *ds = driver->this_->CreateDataSource(datasource_name.c_str(), create_options);

  if (create_options)
    delete [] create_options;

  if (!ds) return NODE_THROW("Error creating datasource.");

  return scope.Close(Datasource::New(ds));
}



Handle<Value> Driver::copyDataSource(const Arguments& args)
{
  HandleScope scope;

  Datasource *source_ds;
  std::string datasource_name;
  Handle<Array> options = Array::New(0);

  NODE_ARG_WRAPPED(0, "data source to copy", Datasource, source_ds);
  NODE_ARG_STR(1, "new datasource name", datasource_name);
  NODE_ARG_ARRAY_OPT(2, "new datasource creation option", options);

  Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

  char **create_options = NULL;

  if (options->Length() > 0) {
    create_options = new char * [options->Length()];
    for (unsigned int i= 0; i < options->Length(); i++) {
      create_options[i] = TOSTR(options->Get(i));
    }
  }

  OGRDataSource *ds = driver->this_->CopyDataSource(source_ds->get(), datasource_name.c_str(), create_options);

  if (create_options)
    delete [] create_options;

  if (!ds) return NODE_THROW("Error copying datasource.");

  return scope.Close(Datasource::New(ds));
}
