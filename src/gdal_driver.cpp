#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"
#include "utils/string_list.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Driver::constructor;
ObjectCache<GDALDriver, Driver> Driver::cache;
#if GDAL_VERSION_MAJOR < 2
ObjectCache<OGRSFDriver, Driver> Driver::cache_ogr;
#endif

void Driver::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Driver::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Driver").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "open", open);
	Nan::SetPrototypeMethod(lcons, "create", create);
	Nan::SetPrototypeMethod(lcons, "createCopy", createCopy);
	Nan::SetPrototypeMethod(lcons, "deleteDataset", deleteDataset);
	Nan::SetPrototypeMethod(lcons, "rename", rename);
	Nan::SetPrototypeMethod(lcons, "copyFiles", copyFiles);
	Nan::SetPrototypeMethod(lcons, "getMetadata", getMetadata);

	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("Driver").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}


#if GDAL_VERSION_MAJOR < 2
Driver::Driver(GDALDriver *driver)
	: Nan::ObjectWrap(), uses_ogr(false), this_gdaldriver(driver), this_ogrdriver(0)
{
	LOG("Created GDAL Driver [%p]", driver);
}
Driver::Driver(OGRSFDriver *driver)
	: Nan::ObjectWrap(), uses_ogr(true), this_gdaldriver(0), this_ogrdriver(driver)
{
	LOG("Created OGR Driver [%p]", driver);
}
Driver::Driver()
	: Nan::ObjectWrap(), uses_ogr(false), this_gdaldriver(0), this_ogrdriver(0)
{
}
#else
Driver::Driver(GDALDriver *driver)
	: Nan::ObjectWrap(), this_gdaldriver(driver)
{
	LOG("Created GDAL Driver [%p]", driver);
}
Driver::Driver()
	: Nan::ObjectWrap(), this_gdaldriver(0)
{
}
#endif



Driver::~Driver()
{
	dispose();
}

void Driver::dispose()
{
	#if GDAL_VERSION_MAJOR < 2
	if(uses_ogr) {
		if(this_ogrdriver) {
			LOG("Disposing OGR Driver [%p]", this_ogrdriver);
			cache_ogr.erase(this_ogrdriver);
			LOG("Disposed OGR Driver [%p]", this_ogrdriver);
			this_ogrdriver = NULL;
		}
		return;
	}
	#endif
	if(this_gdaldriver) {
		LOG("Disposing GDAL Driver [%p]", this_gdaldriver);
		cache.erase(this_gdaldriver);
		LOG("Disposed GDAL Driver [%p]", this_gdaldriver);
		this_gdaldriver = NULL;
	}
}


/**
 * Format specific driver.
 *
 * An instance of this class is created for each supported format, and
 * manages information about the format.
 *
 * This roughly corresponds to a file format, though some drivers may
 * be gateways to many formats through a secondary multi-library.
 *
 * @class gdal.Driver
 */
NAN_METHOD(Driver::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		Driver *f =  static_cast<Driver *>(ptr);
		f->Wrap(info.This());

		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create Driver directly");
		return;
	}
}

Local<Value> Driver::New(GDALDriver *driver)
{
	Nan::EscapableHandleScope scope;

	if (!driver) {
		return scope.Escape(Nan::Null());
	}
	if (cache.has(driver)) {
		return scope.Escape(cache.get(driver));
	}

	Driver *wrapped = new Driver(driver);
	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Driver::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	//LOG("ADDING DRIVER TO CACHE [%p]", driver);
	cache.add(driver, obj);
	//LOG("DONE ADDING DRIVER TO CACHE [%p]", driver);

	return scope.Escape(obj);
}

#if GDAL_VERSION_MAJOR < 2
Local<Value> Driver::New(OGRSFDriver *driver)
{
	Nan::EscapableHandleScope scope;

	if (!driver) {
		return scope.Escape(Nan::Null());
	}
	if (cache_ogr.has(driver)) {
		return scope.Escape(cache_ogr.get(driver));
	}

	Driver *wrapped = new Driver(driver);
	Local<Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(Driver::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	cache_ogr.add(driver, obj);

	return scope.Escape(obj);
}
#endif

NAN_METHOD(Driver::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("Driver").ToLocalChecked());
}

/**
 * @readOnly
 * @attribute description
 * @type String
 */
NAN_GETTER(Driver::descriptionGetter)
{
	Nan::HandleScope scope;
	Driver* driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr) {
		info.GetReturnValue().Set(SafeString::New(driver->getOGRSFDriver()->GetName()));
		return;
	}
	#endif

	info.GetReturnValue().Set(SafeString::New(driver->getGDALDriver()->GetDescription()));
}

/**
 * @throws Error
 * @method deleteDataset
 * @param {string} filename
 */
NAN_METHOD(Driver::deleteDataset)
{
	Nan::HandleScope scope;

	std::string name("");
	NODE_ARG_STR(0, "dataset name", name);

	Driver* driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr) {
		OGRErr err = driver->getOGRSFDriver()->DeleteDataSource(name.c_str());
		if(err) {
			NODE_THROW_OGRERR(err);
			return;
		}
		return;
	}
	#endif

	CPLErr err = driver->getGDALDriver()->Delete(name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}
	return;
}

/**
 * Create a new dataset with this driver.
 *
 * @throws Error
 * @method create
 * @param {String} filename
 * @param {Integer} [x_size=0] raster width in pixels (ignored for vector datasets)
 * @param {Integer} [y_size=0] raster height in pixels (ignored for vector datasets)
 * @param {Integer} [band_count=0]
 * @param {Integer} [data_type=gdal.GDT_Byte] pixel data type (ignored for vector datasets) (see {{#crossLink "Constants (GDT)"}}data types{{/crossLink}})
 * @param {String[]|object} [creation_options] An array or object containing driver-specific dataset creation options
 * @return gdal.Dataset
 */
NAN_METHOD(Driver::create)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	std::string filename;
	unsigned int x_size = 0, y_size = 0, n_bands = 0;
	GDALDataType type = GDT_Byte;
	std::string type_name = "";
	StringList options;

	NODE_ARG_STR(0, "filename", filename);

	if(info.Length() < 3){
		if(info.Length() > 1 && options.parse(info[1])){
			return; //error parsing string list
		}
	} else {
		NODE_ARG_INT(1, "x size", x_size);
		NODE_ARG_INT(2, "y size", y_size);
		NODE_ARG_INT_OPT(3, "number of bands", n_bands);
		NODE_ARG_OPT_STR(4, "data type", type_name);
		if(info.Length() > 5 && options.parse(info[5])){
			return; //error parsing string list
		}
		if(!type_name.empty()) {
			type = GDALGetDataTypeByName(type_name.c_str());
		}
	}


	#if GDAL_VERSION_MAJOR < 2
	if(driver->uses_ogr){
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *ds = raw->CreateDataSource(filename.c_str(), options.get());

		if (!ds) {
			Nan::ThrowError("Error creating dataset");
			return;
		}

		info.GetReturnValue().Set(Dataset::New(ds));
		return;
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALDataset* ds = raw->Create(filename.c_str(), x_size, y_size, n_bands, type, options.get());

	if (!ds) {
		Nan::ThrowError("Error creating dataset");
		return;
	}

	info.GetReturnValue().Set(Dataset::New(ds));
}

/**
 * Create a copy of a dataset.
 *
 * @throws Error
 * @method createCopy
 * @param {String} filename
 * @param {gdal.Dataset} src
 * @param {Boolean} [strict=false]
 * @param {String[]|object} [options=null] An array or object containing driver-specific dataset creation options
 * @return gdal.Dataset
 */
NAN_METHOD(Driver::createCopy)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	if(!driver->isAlive()){
		Nan::ThrowError("Driver object has already been destroyed");
		return;
	}

	std::string filename;
	Dataset* src_dataset;
	unsigned int strict = 0;
	StringList options;

	NODE_ARG_STR(0, "filename", filename);

	//NODE_ARG_STR(1, "source dataset", src_dataset)
	if(info.Length() < 2){
		Nan::ThrowError("source dataset must be provided");
		return;
	}
	if (IS_WRAPPED(info[1], Dataset)) {
		src_dataset = Nan::ObjectWrap::Unwrap<Dataset>(info[1].As<Object>());
	} else {
		Nan::ThrowError("source dataset must be a Dataset object");
		return;
	}

	if(!src_dataset->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	if(info.Length() > 2 && options.parse(info[2])){
		return; //error parsing string list
	}

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr != src_dataset->uses_ogr){
		Nan::ThrowError("Driver unable to copy dataset");
		return;
	}
	if (driver->uses_ogr) {
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *raw_ds = src_dataset->getDatasource();

		OGRDataSource *ds = raw->CopyDataSource(raw_ds, filename.c_str(), options.get());

		if (!ds) {
			Nan::ThrowError("Error copying dataset.");
			return;
		}

		info.GetReturnValue().Set(Dataset::New(ds));
		return;
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALDataset *raw_ds = src_dataset->getDataset();
	GDALDataset *ds = raw->CreateCopy(filename.c_str(), raw_ds, strict, options.get(), NULL, NULL);

	if (!ds) {
		Nan::ThrowError("Error copying dataset");
		return;
	}

	info.GetReturnValue().Set(Dataset::New(ds));
}

/**
 * Copy the files of a dataset.
 *
 * @throws Error
 * @method copyFiles
 * @param {String} name_old New name for the dataset.
 * @param {String} name_new Old name of the dataset.
 */
NAN_METHOD(Driver::copyFiles)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());
	std::string old_name;
	std::string new_name;

	#if GDAL_VERSION_MAJOR < 2
	if(driver->uses_ogr) {
		Nan::ThrowError("Driver unable to copy files");
		return;
	}
	#endif

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->CopyFiles(new_name.c_str(), old_name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Renames the dataset.
 *
 * @throws Error
 * @method rename
 * @param {String} new_name New name for the dataset.
 * @param {String} old_name Old name of the dataset.
 */
NAN_METHOD(Driver::rename)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());
	std::string old_name;
	std::string new_name;

	#if GDAL_VERSION_MAJOR < 2
	if(driver->uses_ogr) {
		Nan::ThrowError("Driver unable to rename files");
		return;
	}
	#endif

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->Rename(new_name.c_str(), old_name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Returns metadata about the driver.
 *
 * @throws Error
 * @method getMetadata
 * @param {String} [domain]
 * @return Object
 */
NAN_METHOD(Driver::getMetadata)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	Local<Object> result;

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr){
		result = Nan::New<Object>();
		result->Set(Nan::New("DCAP_VECTOR").ToLocalChecked(), Nan::New("YES").ToLocalChecked());
		info.GetReturnValue().Set(result);
		return;
	}
	#endif

	GDALDriver* raw = driver->getGDALDriver();
	result = MajorObject::getMetadata(raw, domain.empty() ? NULL : domain.c_str());
	#if GDAL_VERSION_MAJOR < 2
		result->Set(Nan::New("DCAP_RASTER").ToLocalChecked(), Nan::New("YES").ToLocalChecked());
	#endif
	info.GetReturnValue().Set(result);
}

/**
 * Opens a dataset.
 *
 * @throws Error
 * @method open
 * @param {String} path
 * @param {String} [mode=`"r"`] The mode to use to open the file: `"r"` or `"r+"`
 * @return {gdal.Dataset}
 */
NAN_METHOD(Driver::open)
{
	Nan::HandleScope scope;
	Driver *driver = Nan::ObjectWrap::Unwrap<Driver>(info.This());

	std::string path;
	std::string mode = "r";
	GDALAccess access = GA_ReadOnly;

	NODE_ARG_STR(0, "path", path);
	NODE_ARG_OPT_STR(1, "mode", mode);

	if (mode == "r+") {
		access = GA_Update;
	} else if (mode != "r") {
		Nan::ThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr){
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *ds = raw->Open(path.c_str(), static_cast<int>(access));
		if (!ds) {
			Nan::ThrowError("Error opening dataset");
			return;
		}
		info.GetReturnValue().Set(Dataset::New(ds));
		return;
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALOpenInfo *open_info = new GDALOpenInfo(path.c_str(), access);
	GDALDataset *ds = raw->pfnOpen(open_info);
	delete open_info;
	if (!ds) {
		Nan::ThrowError("Error opening dataset");
		return;
	}
	info.GetReturnValue().Set(Dataset::New(ds));
}

} // namespace node_gdal