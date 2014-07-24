#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_rasterband.hpp"
#include "dataset_bands.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> DatasetBands::constructor;

void DatasetBands::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(DatasetBands::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("DatasetBands"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);

	ATTR_DONT_ENUM(constructor, "ds", dsGetter, READ_ONLY_SETTER);

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
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return Null();
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		int band_id;
		NODE_ARG_INT(0, "band id", band_id);
	
		GDALRasterBand *band = raw->GetRasterBand(band_id);

		return scope.Close(RasterBand::New(band));
	}
}

Handle<Value> DatasetBands::create(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	if (ds->uses_ogr){
		return NODE_THROW("Dataset does not support getting creating bands");
	} 

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	GDALDataType type;
	Handle<Array> band_options = Array::New(0);
	char **options = NULL;

	//NODE_ARG_ENUM(0, "data type", GDALDataType, type);
	if(args.Length() < 1) {
		return NODE_THROW("data type argument needed");
	}
	if(args[0]->IsString()){
		type = GDALGetDataTypeByName(TOSTR(args[0]));
	} else if (args[0]->IsNull() || args[0]->IsUndefined()) {
		type = GDT_Unknown;
	} else {
		return NODE_THROW("data type must be string or undefined");
	}

	NODE_ARG_ARRAY_OPT(1, "band creation options", band_options);

	if (band_options->Length() > 0) {
		options = new char* [band_options->Length()];
		for (unsigned int i = 0; i < band_options->Length(); ++i) {
			options[i] = TOSTR(band_options->Get(i));
		}
	}

	CPLErr err = raw->AddBand(type, options);

	if (options) {
		delete [] options;
	}

	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return scope.Close(RasterBand::New(raw->GetRasterBand(raw->GetRasterCount())));
}

Handle<Value> DatasetBands::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return scope.Close(Integer::New(0));
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return scope.Close(Integer::New(raw->GetRasterCount()));
	}
}

Handle<Value> DatasetBands::dsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}

} // namespace node_gdal