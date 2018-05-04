#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_rasterband.hpp"
#include "dataset_bands.hpp"
#include "../utils/string_list.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> DatasetBands::constructor;

void DatasetBands::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(DatasetBands::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("DatasetBands").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "create", create);
	Nan::SetPrototypeMethod(lcons, "get", get);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("DatasetBands").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

DatasetBands::DatasetBands()
	: Nan::ObjectWrap()
{}

DatasetBands::~DatasetBands()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.Dataset"}}Dataset{{/crossLink}}'s raster bands.
 *
 * ```
 * var bands = dataset.bands;```
 *
 * @class gdal.DatasetBands
 */
NAN_METHOD(DatasetBands::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		DatasetBands *f =  static_cast<DatasetBands *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create DatasetBands directly");
		return;
	}
}

Local<Value> DatasetBands::New(Local<Value> ds_obj)
{
	Nan::EscapableHandleScope scope;

	DatasetBands *wrapped = new DatasetBands();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(DatasetBands::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), ds_obj);

	return scope.Escape(obj);
}

NAN_METHOD(DatasetBands::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("DatasetBands").ToLocalChecked());
}

/**
 * Returns the band with the given ID.
 *
 * @method get
 * @param {Integer} id
 * @return {gdal.RasterBand}
 */
NAN_METHOD(DatasetBands::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);

	if (!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr){
		info.GetReturnValue().Set(Nan::Null());
		return;
	} else {
	#else
	{
	#endif
		GDALDataset* raw = ds->getDataset();
		int band_id;
		NODE_ARG_INT(0, "band id", band_id);

		GDALRasterBand *band = raw->GetRasterBand(band_id);

		info.GetReturnValue().Set(RasterBand::New(band, raw));
		return;
	}
}

/**
 * Adds a new band.
 *
 * @method create
 * @throws Error
 * @param {Integer} dataType Type of band ({{#crossLink "Constants (GDT)"}}see GDT constants{{/crossLink}}).
 * @return {gdal.RasterBand}
 */
NAN_METHOD(DatasetBands::create)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);

	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr){
		Nan::ThrowError("Dataset does not support getting creating bands");
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	GDALDataType type;
	StringList options;

	//NODE_ARG_ENUM(0, "data type", GDALDataType, type);
	if(info.Length() < 1) {
		Nan::ThrowError("data type argument needed");
		return;
	}
	if(info[0]->IsString()){
		std::string type_name = *Nan::Utf8String(info[0]);
		type = GDALGetDataTypeByName(type_name.c_str());
	} else if (info[0]->IsNull() || info[0]->IsUndefined()) {
		type = GDT_Unknown;
	} else {
		Nan::ThrowError("data type must be string or undefined");
		return;
	}

	if(info.Length() > 1 && options.parse(info[1])){
		return; //error parsing creation options
	}

	CPLErr err = raw->AddBand(type, options.get());

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	info.GetReturnValue().Set(RasterBand::New(raw->GetRasterBand(raw->GetRasterCount()), raw));
}

/**
 * Returns the number of bands.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(DatasetBands::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);

	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr){
		info.GetReturnValue().Set(Nan::New<Integer>(0));
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	info.GetReturnValue().Set(Nan::New<Integer>(raw->GetRasterCount()));
}

/**
 * Parent dataset
 *
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(DatasetBands::dsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
