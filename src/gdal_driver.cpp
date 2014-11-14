#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"
#include "utils/string_list.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> Driver::constructor;
ObjectCache<GDALDriver, Driver> Driver::cache;
#if GDAL_VERSION_MAJOR < 2
ObjectCache<OGRSFDriver, Driver> Driver::cache_ogr;
#endif

void Driver::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(Driver::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("Driver"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "open", open);
	NODE_SET_PROTOTYPE_METHOD(lcons, "create", create);
	NODE_SET_PROTOTYPE_METHOD(lcons, "createCopy", createCopy);
	NODE_SET_PROTOTYPE_METHOD(lcons, "deleteDataset", deleteDataset);
	NODE_SET_PROTOTYPE_METHOD(lcons, "rename", rename);
	NODE_SET_PROTOTYPE_METHOD(lcons, "copyFiles", copyFiles);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getMetadata", getMetadata);

	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(NanNew("Driver"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}


#if GDAL_VERSION_MAJOR < 2
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
#else
Driver::Driver(GDALDriver *driver)
	: ObjectWrap(), this_gdaldriver(driver)
{
	LOG("Created GDAL Driver [%p]", driver);
}
Driver::Driver()
	: ObjectWrap(), this_gdaldriver(0)
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
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		Driver *f =  static_cast<Driver *>(ptr);
		f->Wrap(args.This());

		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create Driver directly");
		NanReturnUndefined();
	}
}

Handle<Value> Driver::New(GDALDriver *driver)
{
	NanEscapableScope();

	if (!driver) {
		return NanEscapeScope(NanNull());
	}
	if (cache.has(driver)) {
		return NanEscapeScope(NanNew(cache.get(driver)));
	}

	Driver *wrapped = new Driver(driver);
	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Driver::constructor)->GetFunction()->NewInstance(1, &ext);

	cache.add(driver, obj);

	return NanEscapeScope(obj);
}

#if GDAL_VERSION_MAJOR < 2
Handle<Value> Driver::New(OGRSFDriver *driver)
{
	NanEscapableScope();

	if (!driver) {
		return NanEscapeScope(NanNull());
	}
	if (cache_ogr.has(driver)) {
		return NanEscapeScope(NanNew(cache_ogr.get(driver)));
	}

	Driver *wrapped = new Driver(driver);
	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(Driver::constructor)->GetFunction()->NewInstance(1, &ext);

	cache_ogr.add(driver, obj);

	return NanEscapeScope(obj);
}
#endif

NAN_METHOD(Driver::toString)
{
	NanScope();
	NanReturnValue(NanNew("Driver"));
}

/**
 * @readOnly
 * @attribute description
 * @type String
 */
NAN_GETTER(Driver::descriptionGetter)
{
	NanScope();
	Driver* driver = ObjectWrap::Unwrap<Driver>(args.This());

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr) {
		NanReturnValue(SafeString::New(driver->getOGRSFDriver()->GetName()));
	}
	#endif

	NanReturnValue(SafeString::New(driver->getGDALDriver()->GetDescription()));
}

/**
 * @throws Error
 * @method deleteDataset
 * @param {string} filename
 */
NAN_METHOD(Driver::deleteDataset)
{
	NanScope();

	std::string name("");
	NODE_ARG_STR(0, "dataset name", name);

	Driver* driver = ObjectWrap::Unwrap<Driver>(args.This());

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr) {
		OGRErr err = driver->getOGRSFDriver()->DeleteDataSource(name.c_str());
		if(err) {
			NODE_THROW_OGRERR(err);
			NanReturnUndefined();
		}
		NanReturnUndefined();
	}
	#endif

	CPLErr err = driver->getGDALDriver()->Delete(name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string filename;
	unsigned int x_size = 0, y_size = 0, n_bands = 0;
	GDALDataType type = GDT_Byte;
	std::string type_name = "";
	StringList options;

	NODE_ARG_STR(0, "filename", filename);

	if(args.Length() < 3){
		if(args.Length() > 1 && options.parse(args[1])){
			NanReturnUndefined(); //error parsing string list
		}
	} else {
		NODE_ARG_INT(1, "x size", x_size);
		NODE_ARG_INT(2, "y size", y_size);
		NODE_ARG_INT_OPT(3, "number of bands", n_bands);
		NODE_ARG_OPT_STR(4, "data type", type_name);
		if(args.Length() > 5 && options.parse(args[5])){
			NanReturnUndefined(); //error parsing string list
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
			NanThrowError("Error creating dataset");
			NanReturnUndefined();
		}

		NanReturnValue(Dataset::New(ds));
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALDataset* ds = raw->Create(filename.c_str(), x_size, y_size, n_bands, type, options.get());

	if (!ds) {
		NanThrowError("Error creating dataset");
		NanReturnUndefined();
	}

	NanReturnValue(Dataset::New(ds));
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string filename;
	Dataset* src_dataset;
	unsigned int strict = 0;
	StringList options;

	NODE_ARG_STR(0, "filename", filename);

	//NODE_ARG_STR(1, "source dataset", src_dataset)
	if(args.Length() < 2){
		NanThrowError("source dataset must be provided");
		NanReturnUndefined();
	}
	if (IS_WRAPPED(args[1], Dataset)) {
		src_dataset = ObjectWrap::Unwrap<Dataset>(args[1].As<Object>());
	} else {
		NanThrowError("source dataset must be a Dataset object");
		NanReturnUndefined();
	}


	if(args.Length() > 2 && options.parse(args[2])){
		NanReturnUndefined(); //error parsing string list
	}

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr) {
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *raw_ds = src_dataset->getDatasource();
		if(!src_dataset->uses_ogr) {
			NanThrowError("Driver unable to copy dataset");
			NanReturnUndefined();
		}
		if (!raw_ds) {
			NanThrowError("Dataset object has already been destroyed");
			NanReturnUndefined();
		}

		OGRDataSource *ds = raw->CopyDataSource(raw_ds, filename.c_str(), options.get());

		if (!ds) {
			NanThrowError("Error copying dataset.");
			NanReturnUndefined();
		}

		NanReturnValue(Dataset::New(ds));
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALDataset* raw_ds = src_dataset->getDataset();
	if(!raw_ds) {
		NanThrowError("Dataset object has already been destroyed");
		NanReturnUndefined();
	}
	GDALDataset* ds = raw->CreateCopy(filename.c_str(), raw_ds, strict, options.get(), NULL, NULL);

	if (!ds) {
		NanThrowError("Error copying dataset");
		NanReturnUndefined();
	}

	NanReturnValue(Dataset::New(ds));
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
	std::string old_name;
	std::string new_name;

	#if GDAL_VERSION_MAJOR < 2
	if(driver->uses_ogr) {
		NanThrowError("Driver unable to copy files");
		NanReturnUndefined();
	}
	#endif

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->CopyFiles(new_name.c_str(), old_name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());
	std::string old_name;
	std::string new_name;

	#if GDAL_VERSION_MAJOR < 2
	if(driver->uses_ogr) {
		NanThrowError("Driver unable to rename files");
		NanReturnUndefined();
	}
	#endif

	NODE_ARG_STR(0, "new name", new_name);
	NODE_ARG_STR(1, "old name", old_name);

	CPLErr err = driver->getGDALDriver()->Rename(new_name.c_str(), old_name.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	Handle<Object> result;

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr){
		result = NanNew<Object>();
		result->Set(NanNew("DCAP_VECTOR"), NanNew("YES"));
		NanReturnValue(result);
	}
	#endif

	GDALDriver* raw = driver->getGDALDriver();
	result = MajorObject::getMetadata(raw, domain.empty() ? NULL : domain.c_str());
	#if GDAL_VERSION_MAJOR < 2
		result->Set(NanNew("DCAP_RASTER"), NanNew("YES"));
	#endif
	NanReturnValue(result);
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
	NanScope();
	Driver *driver = ObjectWrap::Unwrap<Driver>(args.This());

	std::string path;
	std::string mode = "r";
	GDALAccess access = GA_ReadOnly;

	NODE_ARG_STR(0, "path", path);
	NODE_ARG_OPT_STR(1, "mode", mode);

	if (mode == "r+") {
		access = GA_Update;
	} else if (mode != "r") {
		NanThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
		NanReturnUndefined();
	}

	#if GDAL_VERSION_MAJOR < 2
	if (driver->uses_ogr){
		OGRSFDriver *raw = driver->getOGRSFDriver();
		OGRDataSource *ds = raw->Open(path.c_str(), static_cast<int>(access));
		if (!ds) {
			NanThrowError("Error opening dataset");
			NanReturnUndefined();
		}
		NanReturnValue(Dataset::New(ds));
	}
	#endif

	GDALDriver *raw = driver->getGDALDriver();
	GDALOpenInfo *info = new GDALOpenInfo(path.c_str(), access);
	GDALDataset *ds = raw->pfnOpen(info);
	delete info;
	if (!ds) {
		NanThrowError("Error opening dataset");
		NanReturnUndefined();
	}
	NanReturnValue(Dataset::New(ds));
}

} // namespace node_gdal