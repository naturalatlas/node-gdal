#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_rasterband.hpp"
#include "dataset_bands.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> DatasetBands::constructor;

void DatasetBands::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(DatasetBands::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("DatasetBands"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "create", create);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(NanNew("DatasetBands"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

DatasetBands::DatasetBands()
	: ObjectWrap()
{}

DatasetBands::~DatasetBands() 
{}

NAN_METHOD(DatasetBands::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		DatasetBands *f =  static_cast<DatasetBands *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create DatasetBands directly");
		NanReturnUndefined();
	}
}

Handle<Value> DatasetBands::New(Handle<Value> ds_obj)
{
	NanEscapableScope();

	DatasetBands *wrapped = new DatasetBands();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(DatasetBands::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), ds_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(DatasetBands::toString)
{
	NanScope();
	NanReturnValue(NanNew("DatasetBands"));
}

NAN_METHOD(DatasetBands::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			NanThrowError("Dataset object has already been destroyed");
			NanReturnUndefined();
		}
		return NanNull();
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			NanThrowError("Dataset object has already been destroyed");
			NanReturnUndefined();
		}
		int band_id;
		NODE_ARG_INT(0, "band id", band_id);
	
		GDALRasterBand *band = raw->GetRasterBand(band_id);

		NanReturnValue(RasterBand::New(band, raw));
	}
}

NAN_METHOD(DatasetBands::create)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	if (ds->uses_ogr){
		NanThrowError("Dataset does not support getting creating bands");
		NanReturnUndefined();
	} 

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		NanThrowError("Dataset object has already been destroyed");
		NanReturnUndefined();
	}

	GDALDataType type;
	Handle<Array> band_options = NanNew<Array>(0);
	char **options = NULL;
	std::string *options_str = NULL;

	//NODE_ARG_ENUM(0, "data type", GDALDataType, type);
	if(args.Length() < 1) {
		NanThrowError("data type argument needed");
		NanReturnUndefined();
	}
	if(args[0]->IsString()){
		NanUtf8String type_name = NanUtf8String(args[0]);
		type = GDALGetDataTypeByName(*type_name);
	} else if (args[0]->IsNull() || args[0]->IsUndefined()) {
		type = GDT_Unknown;
	} else {
		NanThrowError("data type must be string or undefined");
		NanReturnUndefined();
	}

	NODE_ARG_ARRAY_OPT(1, "band creation options", band_options);

	if (band_options->Length() > 0) {
		options     = new char* [band_options->Length()];
		options_str = new std::string [band_options->Length()];
		for (unsigned int i = 0; i < band_options->Length(); ++i) {
			options_str[i] = *NanUtf8String(band_options->Get(i));
			options[i] = (char*) options_str[i].c_str();
		}
	}

	CPLErr err = raw->AddBand(type, options);

	if(options)	    delete [] options;
	if(options_str)	delete [] options_str;
	
	if(err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(RasterBand::New(raw->GetRasterBand(raw->GetRasterCount()), raw));
}

NAN_METHOD(DatasetBands::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			NanThrowError("Dataset object has already been destroyed");
			NanReturnUndefined();
		}
		NanReturnValue(NanNew<Integer>(0));
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			NanThrowError("Dataset object has already been destroyed");
			NanReturnUndefined();
		}
		NanReturnValue(NanNew<Integer>(raw->GetRasterCount()));
	}
}

NAN_GETTER(DatasetBands::dsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal