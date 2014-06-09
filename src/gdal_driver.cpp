#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"

Persistent<FunctionTemplate> Driver::constructor;

void Driver::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Driver::New));
	constructor->Inherit(MajorObject::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Driver"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createCopy", createCopy);
	NODE_SET_PROTOTYPE_METHOD(constructor, "deleteDataset", deleteDataset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "quietDelete", quietDelete);
	NODE_SET_PROTOTYPE_METHOD(constructor, "rename", rename);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copyFiles", copyFiles);

	ATTR(constructor, "ShortName", shortNameGetter, NULL);
	ATTR(constructor, "LongName", longNameGetter, NULL);

	target->Set(String::NewSymbol("Driver"), constructor->GetFunction());
}

Driver::Driver(GDALDriver *ds)
	: ObjectWrap(), this_(ds)
{}

Driver::Driver()
	: ObjectWrap(), this_(0)
{
}

Driver::~Driver()
{
}



Handle<Value> Driver::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		Driver *f =  static_cast<Driver *>(ptr);
		f->Wrap(args.This());

		return args.This();
	} else {
		return NODE_THROW("Cannot create Driver directly");
	}
}

Handle<Value> Driver::New(GDALDriver *driver)
{
	if (!driver) {
		return Null();
	}

	v8::HandleScope scope;
	Driver *wrapped = new Driver(driver);
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Driver::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> Driver::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Driver"));
}

Handle<Value> Driver::shortNameGetter(Local<String> property, const AccessorInfo& info)
{
	HandleScope scope;
	Driver* driver = ObjectWrap::Unwrap<Driver>(info.This());
	return scope.Close(SafeString::New(driver->this_->GetDescription()));
}

Handle<Value> Driver::longNameGetter(Local<String> property, const AccessorInfo& info)
{
	HandleScope scope;
	Driver* driver = ObjectWrap::Unwrap<Driver>(info.This());
	return scope.Close(SafeString::New(driver->this_->GetMetadataItem(GDAL_DMD_LONGNAME)));
}

NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_STRING_PARAM(Driver, deleteDataset, Delete, "dataset name");
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_STRING_PARAM(Driver, quietDelete, QuietDelete, "dataset name");

Handle<Value> Driver::create(const Arguments& args)
{
	HandleScope scope;
	std::string filename;
	unsigned int i, x_size, y_size, n_bands = 1;
	GDALDataType type = GDT_Byte;
	Handle<Array> dataset_options = Array::New(0);

	NODE_ARG_STR(0, "filename", filename);
	NODE_ARG_INT(1, "x size", x_size);
	NODE_ARG_INT(2, "y size", y_size);
	NODE_ARG_INT_OPT(3, "number of bands", n_bands);
	NODE_ARG_ENUM_OPT(4, "data type", GDALDataType, type);
	NODE_ARG_ARRAY_OPT(5, "dataset creation options", dataset_options);

	char **options = NULL;

	if (dataset_options->Length() > 0) {
		options = new char* [dataset_options->Length() + 1];
		for (i = 0; i < dataset_options->Length(); ++i) {
			options[i] = TOSTR(dataset_options->Get(i));
		}
		options[i] = NULL;
	}

	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	GDALDataset* dataset = driver->this_->Create(filename.c_str(), x_size, y_size, n_bands, type, options);

	if (options) {
		delete [] options;
	}

	if (!dataset) {
		return NODE_THROW("Error creating dataset");
	}

	return scope.Close(Dataset::New(dataset));
}
Handle<Value> Driver::createCopy(const Arguments& args)
{
	HandleScope scope;
	std::string filename;
	Dataset* src_dataset;
	unsigned int i, strict = 0;
	Handle<Array> dataset_options = Array::New(0);

	NODE_ARG_STR(0, "filename", filename);
	NODE_ARG_WRAPPED(1, "source dataset", Dataset, src_dataset);
	NODE_ARG_BOOL_OPT(2, "strict", strict);
	NODE_ARG_ARRAY_OPT(3, "dataset creation options", dataset_options);
	//todo: add optional progress callback argument

	char **options = NULL;

	if (dataset_options->Length() > 0) {
		options = new char* [dataset_options->Length()+1];
		for (i = 0; i < dataset_options->Length(); ++i) {
			options[i] = TOSTR(dataset_options->Get(i));
		}
		options[i] = NULL;
	}

	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	GDALDataset* dataset = driver->this_->CreateCopy(filename.c_str(), src_dataset->get(), strict, options, NULL, NULL);

	if (options) {
		delete [] options;
	}

	if (!dataset) {
		return NODE_THROW("Error copying dataset");
	}

	return scope.Close(Dataset::New(dataset));
}

Handle<Value> Driver::copyFiles(const Arguments& args)
{
	std::string old_name;
	std::string new_name;

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	CPLErr err = driver->this_->CopyFiles(new_name.c_str(), old_name.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Driver::rename(const Arguments& args)
{
	HandleScope scope;

	std::string old_name;
	std::string new_name;

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	CPLErr err = driver->this_->Rename(new_name.c_str(), old_name.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}
