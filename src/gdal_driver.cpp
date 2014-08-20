#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> Driver::constructor;
ObjectCache<GDALDriver*> Driver::cache;
ObjectCache<OGRSFDriver*> Driver::cache_ogr;

void Driver::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Driver::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Driver"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "open", open);
	NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createCopy", createCopy);
	NODE_SET_PROTOTYPE_METHOD(constructor, "deleteDataset", deleteDataset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "rename", rename);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copyFiles", copyFiles);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMetadata", getMetadata);

	ATTR(constructor, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("Driver"), constructor->GetFunction());
}

Driver::Driver(GDALDriver *driver)
	: ObjectWrap(), uses_ogr(false), this_gdaldriver(driver), this_ogrdriver(0)
{
	LOG("Created GDAL Driver [%p]", driver);
}

Driver::Driver(OGRSFDriver *driver)
	: ObjectWrap(), uses_ogr(true), this_gdaldriver(0), this_ogrdriver(driver)
{
	LOG("Created OGR Driver [%p]", driver);
}

Driver::Driver()
	: ObjectWrap(), uses_ogr(false), this_gdaldriver(0), this_ogrdriver(0)
{
}

Driver::~Driver()
{
	if(uses_ogr) {
		LOG("Disposing OGR Driver [%p]", this_ogrdriver);
		if(this_ogrdriver) {
			cache_ogr.erase(this_ogrdriver);
			this_ogrdriver = NULL;
		}
		LOG("Disposed OGR Driver [%p]", this_ogrdriver);
	} else {
		LOG("Disposing GDAL Driver [%p]", this_gdaldriver);
		if(this_gdaldriver) {
			cache.erase(this_gdaldriver);
			this_gdaldriver = NULL;
		}
		LOG("Disposed GDAL Driver [%p]", this_gdaldriver);
	}

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
	HandleScope scope;

	if (!driver) {
		return Null();
	}
	if (cache.has(driver)) {
		return cache.get(driver);
	}

	Driver *wrapped = new Driver(driver);
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Driver::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(driver, obj);

	return scope.Close(obj);
}

Handle<Value> Driver::New(OGRSFDriver *driver)
{
	HandleScope scope;

	if (!driver) {
		return Null();
	}
	if (cache_ogr.has(driver)) {
		return cache_ogr.get(driver);
	}

	Driver *wrapped = new Driver(driver);
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Driver::constructor->GetFunction()->NewInstance(1, &ext);

	cache_ogr.add(driver, obj);

	return scope.Close(obj);
}

Handle<Value> Driver::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Driver"));
}

Handle<Value> Driver::descriptionGetter(Local<String> property, const AccessorInfo& info)
{
	HandleScope scope;
	Driver* driver = ObjectWrap::Unwrap<Driver>(info.This());
	if (driver->uses_ogr) {
		return scope.Close(SafeString::New(driver->getOGRSFDriver()->GetName()));
	} else {
		return scope.Close(SafeString::New(driver->getGDALDriver()->GetDescription()));
	}
}

Handle<Value> Driver::deleteDataset(const Arguments& args)
{
	HandleScope scope;

	std::string name("");
	NODE_ARG_STR(0, "dataset name", name);

	Driver* driver = ObjectWrap::Unwrap<Driver>(args.This());
	if (driver->uses_ogr) {
		OGRErr err = driver->getOGRSFDriver()->DeleteDataSource(name.c_str());
		if (err) {
			return NODE_THROW_OGRERR(err);
		} 
	} else {
		CPLErr err = driver->getGDALDriver()->Delete(name.c_str());
		if (err) {
			return NODE_THROW_CPLERR(err);
		} 
	}
	return Undefined();
}

Handle<Value> Driver::create(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string filename;
	unsigned int i, x_size = 0, y_size = 0, n_bands = 1;
	GDALDataType type = GDT_Byte;
	Handle<Array> creation_options = Array::New(0);


	NODE_ARG_STR(0, "filename", filename);
	
	if(args.Length() < 3){
		NODE_ARG_ARRAY_OPT(1, "creation options", creation_options);
	} else {
		NODE_ARG_INT(1, "x size", x_size);
		NODE_ARG_INT(2, "y size", y_size);
		NODE_ARG_INT_OPT(3, "number of bands", n_bands);
		NODE_ARG_ENUM_OPT(4, "data type", GDALDataType, type);
		NODE_ARG_ARRAY_OPT(5, "creation options", creation_options);
	}

	char **options = NULL;
	std::string *options_str;
	if (creation_options->Length() > 0) {
		options = new char* [creation_options->Length() + 1];
		options_str = new std::string [creation_options->Length() + 1];
		for (i = 0; i < creation_options->Length(); ++i) {
			options_str[i] = TOSTR(creation_options->Get(i));
			options[i]     = (char*) options_str[i].c_str();
		}
		options[i] = NULL;
	}

	if(driver->uses_ogr){
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *ds = raw->CreateDataSource(filename.c_str(), options);

		if (options) {
			delete [] options;
			delete [] options_str;
		}

		if (!ds) {
			return NODE_THROW("Error creating dataset");
		}

		return scope.Close(Dataset::New(ds));
	} else {
		GDALDriver *raw = driver->getGDALDriver();
		GDALDataset* ds = raw->Create(filename.c_str(), x_size, y_size, n_bands, type, options);

		if (options) {
			delete [] options;
			delete [] options_str;
		}

		if (!ds) {
			return NODE_THROW("Error creating dataset");
		}

		return scope.Close(Dataset::New(ds));
	}

}
Handle<Value> Driver::createCopy(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string filename;
	Dataset* src_dataset;
	unsigned int i, strict = 0;
	Handle<Array> creation_options = Array::New(0);

	NODE_ARG_STR(0, "filename", filename);

	//NODE_ARG_STR(1, "source dataset", src_dataset)
	if(args.Length() < 2){
		return NODE_THROW("source dataset must be provided");
	}
	if (IS_WRAPPED(args[1], Dataset)) {
		src_dataset = ObjectWrap::Unwrap<Dataset>(args[1]->ToObject());
	} else {
		return NODE_THROW("source dataset must be a Dataset object")
	}	

	NODE_ARG_ARRAY_OPT(2, "dataset creation options", creation_options);

	char **options = NULL;
	std::string *options_str = NULL;

	if (creation_options->Length() > 0) {
		options = new char* [creation_options->Length()+1];
		options_str = new std::string [creation_options->Length()+1];
		for (i = 0; i < creation_options->Length(); ++i) {
			options_str[i] = TOSTR(creation_options->Get(i));
			options[i]     = (char*) options_str[i].c_str();
		}
		options[i] = NULL;
	}

	if (driver->uses_ogr) {
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *raw_ds = src_dataset->getDatasource();
		if(!src_dataset->uses_ogr) {
			return NODE_THROW("Driver unable to copy dataset");
		}
		if (!raw_ds) {
			return NODE_THROW("Dataset object has already been destroyed");
		}

		OGRDataSource *ds = raw->CopyDataSource(raw_ds, filename.c_str(), options);

		if(options)	    delete [] options;
		if(options_str)	delete [] options_str;

		if (!ds) {
			return NODE_THROW("Error copying dataset.");
		}

		return scope.Close(Dataset::New(ds));
	} else {
		GDALDriver *raw = driver->getGDALDriver();
		GDALDataset* raw_ds = src_dataset->getDataset();
		if(src_dataset->uses_ogr) {
			return NODE_THROW("Driver unable to copy dataset");
		}
		if(!raw_ds) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		GDALDataset* ds = raw->CreateCopy(filename.c_str(), raw_ds, strict, options, NULL, NULL);

		if(options)	    delete [] options;
		if(options_str)	delete [] options_str;

		if (!ds) {
			return NODE_THROW("Error copying dataset");
		}

		return scope.Close(Dataset::New(ds));
	}
}

Handle<Value> Driver::copyFiles(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
	std::string old_name;
	std::string new_name;

	if(driver->uses_ogr) {
		return NODE_THROW("Driver unable to copy files");
	}

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->CopyFiles(new_name.c_str(), old_name.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Driver::rename(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
	std::string old_name;
	std::string new_name;

	if(driver->uses_ogr) {
		return NODE_THROW("Driver unable to rename files");
	}

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->Rename(new_name.c_str(), old_name.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Driver::getMetadata(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
	
	Handle<Object> result; 

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	if (driver->uses_ogr){
		result = Object::New();
		result->Set(String::NewSymbol("DCAP_VECTOR"), String::New("YES"));
		return scope.Close(result);
	}

	GDALDriver* raw = driver->getGDALDriver();
	result = MajorObject::getMetadata(raw, domain.empty() ? NULL : domain.c_str());
	#if GDAL_MAJOR < 2
		result->Set(String::NewSymbol("DCAP_RASTER"), String::New("YES"));
	#endif
	return scope.Close(result);
}

Handle<Value> Driver::open(const Arguments& args)
{
	HandleScope scope;
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string path;
	std::string mode = "r";
	GDALAccess access = GA_ReadOnly;

	NODE_ARG_STR(0, "path", path);
	NODE_ARG_OPT_STR(1, "mode", mode);

	if (mode == "r+") {
		access = GA_Update;
	} else if (mode != "r") {
		return NODE_THROW("Invalid open mode. Must be \"r\" or \"r+\"");
	}

	if (driver->uses_ogr){
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *ds = raw->Open(path.c_str(), static_cast<int>(access));
		if (!ds) {
			return NODE_THROW("Error opening dataset");
		}
		return scope.Close(Dataset::New(ds));
	} else {
		GDALDriver  *raw = driver->getGDALDriver();
		GDALOpenInfo *info = new GDALOpenInfo(path.c_str(), access);
		GDALDataset *ds = raw->pfnOpen(info);
		delete info;
		if (!ds) {
			return NODE_THROW("Error opening dataset");
		}
		return scope.Close(Dataset::New(ds));
	}
}

} // namespace node_gdal