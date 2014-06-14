#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_rasterband.hpp"
#include "dataset_bands.hpp"

Persistent<FunctionTemplate> DatasetBands::constructor;

void DatasetBands::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(DatasetBands::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("DatasetBands"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);

	ATTR(constructor, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("DatasetBands"), constructor->GetFunction());
}

DatasetBands::DatasetBands()
	: ObjectWrap()
{}

DatasetBands::~DatasetBands() 
{}

Handle<Value> DatasetBands::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		DatasetBands *f =  static_cast<DatasetBands *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create DatasetBands directly");
	}
}

Handle<Value> DatasetBands::New(Handle<Value> ds_obj)
{
	HandleScope scope;

	DatasetBands *wrapped = new DatasetBands();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = DatasetBands::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), ds_obj);

	return scope.Close(obj);
}

Handle<Value> DatasetBands::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("DatasetBands"));
}

Handle<Value> DatasetBands::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	if (!ds->get()) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	int band_id;
	NODE_ARG_INT(0, "band id", band_id);
	
	GDALRasterBand *band = ds->get()->GetRasterBand(band_id);

	if (band == NULL) {
		return NODE_THROW("Specified band not found");
	}

	return scope.Close(RasterBand::New(band));
}

Handle<Value> DatasetBands::create(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	if (!ds->get()) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	GDALDataType type;
	Handle<Array> band_options = Array::New(0);
	char **options = NULL;

	NODE_ARG_ENUM(0, "data type", GDALDataType, type);
	NODE_ARG_ARRAY_OPT(1, "band creation options", band_options);

	if (band_options->Length() > 0) {
		options = new char* [band_options->Length()];
		for (unsigned int i = 0; i < band_options->Length(); ++i) {
			options[i] = TOSTR(band_options->Get(i));
		}
	}

	CPLErr err = ds->get()->AddBand(type, options);

	if (options) {
		delete [] options;
	}

	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return scope.Close(RasterBand::New(ds->get()->GetRasterBand(ds->get()->GetRasterCount())));
}

Handle<Value> DatasetBands::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	if (!ds->get()) {
		return NODE_THROW("Dataset object has already been destroyed");
	}
	
	return scope.Close(Integer::New(ds->get()->GetRasterCount()));
}

Handle<Value> DatasetBands::dsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}