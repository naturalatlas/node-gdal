#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_driver.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_layer.hpp"
#include "gdal_geometry.hpp"
#include "collections/dataset_bands.hpp"
#include "collections/dataset_layers.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> Dataset::constructor;
ObjectCache<GDALDataset, Dataset> Dataset::dataset_cache;
#if GDAL_VERSION_MAJOR < 2
ObjectCache<OGRDataSource, Dataset> Dataset::datasource_cache;
#endif

void Dataset::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Dataset::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("Dataset").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "setGCPs", setGCPs);
	Nan::SetPrototypeMethod(lcons, "getGCPs", getGCPs);
	Nan::SetPrototypeMethod(lcons, "getGCPProjection", getGCPProjection);
	Nan::SetPrototypeMethod(lcons, "getFileList", getFileList);
	Nan::SetPrototypeMethod(lcons, "flush", flush);
	Nan::SetPrototypeMethod(lcons, "close", close);
	Nan::SetPrototypeMethod(lcons, "getMetadata", getMetadata);
	Nan::SetPrototypeMethod(lcons, "testCapability", testCapability);
	Nan::SetPrototypeMethod(lcons, "executeSQL", executeSQL);
	Nan::SetPrototypeMethod(lcons, "buildOverviews", buildOverviews);

	ATTR_DONT_ENUM(lcons, "_uid", uidGetter, READ_ONLY_SETTER);
	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);
	ATTR(lcons, "bands", bandsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "layers", layersGetter, READ_ONLY_SETTER);
	ATTR(lcons, "rasterSize", rasterSizeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "driver", driverGetter, READ_ONLY_SETTER);
	ATTR(lcons, "srs", srsGetter, srsSetter);
	ATTR(lcons, "geoTransform", geoTransformGetter, geoTransformSetter);

	target->Set(Nan::New("Dataset").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}



#if GDAL_VERSION_MAJOR < 2
Dataset::Dataset(GDALDataset *ds)
	: Nan::ObjectWrap(),
	  uid(0),
	  uses_ogr(false),
	  this_dataset(ds),
	  this_datasource(0)
{
	LOG("Created Dataset [%p]", ds);
}
Dataset::Dataset(OGRDataSource *ds)
	: Nan::ObjectWrap(),
	  uid(0),
	  uses_ogr(true),
	  this_dataset(0),
	  this_datasource(ds)
{
	LOG("Created Datasource [%p]", ds);
}
#else
Dataset::Dataset(GDALDataset *ds)
	: Nan::ObjectWrap(),
	  uid(0),
	  this_dataset(ds)
{
	LOG("Created Dataset [%p]", ds);
}
#endif

Dataset::~Dataset()
{
	//Destroy at garbage collection time if not already explicitly destroyed
	dispose();
}

void Dataset::dispose()
{

	#if GDAL_VERSION_MAJOR < 2

	if (this_datasource) {
		LOG("Disposing Datasource [%p]", this_datasource);

		ptr_manager.dispose(uid);

		LOG("Disposed Datasource [%p]", this_datasource);

		this_datasource = NULL;
	}
	#endif

	if (this_dataset) {
		LOG("Disposing Dataset [%p]", this_dataset);

		ptr_manager.dispose(uid);

		LOG("Disposed Dataset [%p]", this_dataset);

		this_dataset = NULL;
	}
}

/**
 * A set of associated raster bands and/or vector layers, usually from one file.
 *
 * ```
 * // raster dataset:
 * dataset = gdal.open('file.tif');
 * bands = dataset.bands;
 *
 * // vector dataset:
 * dataset = gdal.open('file.shp');
 * layers = dataset.layers;```
 *
 * @class gdal.Dataset
 */
NAN_METHOD(Dataset::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		Dataset *f =  static_cast<Dataset *>(ptr);
		f->Wrap(info.This());

		Local<Value> bands = DatasetBands::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("bands_").ToLocalChecked(), bands);

		Local<Value> layers = DatasetLayers::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("layers_").ToLocalChecked(), layers);

		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create dataset directly");
		return;
	}
}

Local<Value> Dataset::New(GDALDataset *raw)
{
	Nan::EscapableHandleScope scope;

	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	if (dataset_cache.has(raw)) {
		return scope.Escape(dataset_cache.get(raw));
	}

	Dataset *wrapped = new Dataset(raw);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(Dataset::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	dataset_cache.add(raw, obj);
	wrapped->uid = ptr_manager.add(raw);

	return scope.Escape(obj);
}

#if GDAL_VERSION_MAJOR < 2
Local<Value> Dataset::New(OGRDataSource *raw)
{
	Nan::EscapableHandleScope scope;

	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	if (datasource_cache.has(raw)) {
		return scope.Escape(datasource_cache.get(raw));
	}

	Dataset *wrapped = new Dataset(raw);

	Local<Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(Dataset::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	datasource_cache.add(raw, obj);
	wrapped->uid = ptr_manager.add(raw);

	return scope.Escape(obj);
}
#endif

NAN_METHOD(Dataset::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("Dataset").ToLocalChecked());
}

/**
 * Fetch metadata.
 *
 * @method getMetadata
 * @param {string} [domain]
 * @return {Object}
 */
NAN_METHOD(Dataset::getMetadata)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(Nan::New<Object>());
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);
	info.GetReturnValue().Set(MajorObject::getMetadata(raw, domain.empty() ? NULL : domain.c_str()));
}

/**
 * Determines if the dataset supports the indicated operation.
 *
 * @method testCapability
 * @param {string} capability (see {{#crossLink "Constants (ODsC)"}}capability list{{/crossLink}})
 * @return {Boolean}
 */
NAN_METHOD(Dataset::testCapability)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			info.GetReturnValue().Set(Nan::False());
			return;
		}
	#endif

	std::string capability("");
	NODE_ARG_STR(0, "capability", capability);

	info.GetReturnValue().Set(Nan::New<Boolean>(raw->TestCapability(capability.c_str())));
}

/**
 * Get output projection for GCPs.
 *
 * @method getGCPProjection
 * @return {String}
 */
NAN_METHOD(Dataset::getGCPProjection)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	info.GetReturnValue().Set(SafeString::New(raw->GetGCPProjection()));
}

/**
 * Closes the dataset to further operations.
 *
 * @method close
 */
NAN_METHOD(Dataset::close)
{
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	ds->dispose();

	return;
}

/**
 * Flushes all changes to disk.
 *
 * @throws Error
 * @method flush
 */
NAN_METHOD(Dataset::flush)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		OGRErr err = raw->SyncToDisk();
		if(err) {
			NODE_THROW_OGRERR(err);
			return;
		}
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}
	raw->FlushCache();

	return;
}

/**
 * Execute an SQL statement against the data store.
 *
 * @throws Error
 * @method executeSQL
 * @param {String} statement SQL statement to execute.
 * @param {gdal.Geometry} [spatial_filter=null] Geometry which represents a spatial filter.
 * @param {String} [dialect=null] Allows control of the statement dialect. If set to `null`, the OGR SQL engine will be used, except for RDBMS drivers that will use their dedicated SQL engine, unless `"OGRSQL"` is explicitely passed as the dialect. Starting with OGR 1.10, the `"SQLITE"` dialect can also be used.
 * @return {gdal.Layer}
 */
NAN_METHOD(Dataset::executeSQL)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset* raw = ds->getDataset();
	#else
		OGRDataSource* raw = ds->getDatasource();
		if (!ds->uses_ogr){
			Nan::ThrowError("Dataset does not support executing a SQL query");
			return;
		}
	#endif

	std::string sql;
	std::string sql_dialect;
	Geometry *spatial_filter = NULL;

	NODE_ARG_STR(0, "sql text", sql);
	NODE_ARG_WRAPPED_OPT(1, "spatial filter geometry", Geometry, spatial_filter);
	NODE_ARG_OPT_STR(2, "sql dialect", sql_dialect);

	OGRLayer *layer = raw->ExecuteSQL(sql.c_str(),
											spatial_filter ? spatial_filter->get() : NULL,
											sql_dialect.empty() ? NULL : sql_dialect.c_str());

	if (layer) {
		info.GetReturnValue().Set(Layer::New(layer, raw, true));
		return;
	} else {
		Nan::ThrowError("Error executing SQL");
		return;
	}
}

/**
 * Fetch files forming dataset.
 *
 * Returns a list of files believed to be part of this dataset. If it returns an
 * empty list of files it means there is believed to be no local file system files
 * associated with the dataset (for instance a virtual dataset).
 *
 * Returns an empty array for vector datasets if GDAL version is below 2.0
 *
 * @method getFileList
 * @return {String[]}
 */
NAN_METHOD(Dataset::getFileList)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	Local<Array> results = Nan::New<Array>(0);

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(results);
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	char **list = raw->GetFileList();
	if (!list) {
		info.GetReturnValue().Set(results);
		return;
	}

	int i = 0;
	while (list[i]) {
		results->Set(i, SafeString::New(list[i]));
		i++;
	}

	CSLDestroy(list);

	info.GetReturnValue().Set(results);
}

/**
 * Fetches GCPs.
 *
 * @method getGCPs
 * @return {Object[]}
 */
NAN_METHOD(Dataset::getGCPs)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	Local<Array> results = Nan::New<Array>(0);

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(results);
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	int n = raw->GetGCPCount();
	const GDAL_GCP *gcps = raw->GetGCPs();

	if (!gcps) {
		info.GetReturnValue().Set(results);
		return;
	}

	for (int i = 0; i < n; i++) {
		GDAL_GCP gcp = gcps[i];
		Local<Object> obj = Nan::New<Object>();
		obj->Set(Nan::New("pszId").ToLocalChecked(), Nan::New(gcp.pszId).ToLocalChecked());
		obj->Set(Nan::New("pszInfo").ToLocalChecked(), Nan::New(gcp.pszInfo).ToLocalChecked());
		obj->Set(Nan::New("dfGCPPixel").ToLocalChecked(), Nan::New<Number>(gcp.dfGCPPixel));
		obj->Set(Nan::New("dfGCPLine").ToLocalChecked(), Nan::New<Number>(gcp.dfGCPLine));
		obj->Set(Nan::New("dfGCPX").ToLocalChecked(), Nan::New<Number>(gcp.dfGCPX));
		obj->Set(Nan::New("dfGCPY").ToLocalChecked(), Nan::New<Number>(gcp.dfGCPY));
		obj->Set(Nan::New("dfGCPZ").ToLocalChecked(), Nan::New<Number>(gcp.dfGCPZ));
		results->Set(i, obj);
	}

	info.GetReturnValue().Set(results);
}

/**
 * Sets GCPs.
 *
 * @throws Error
 * @method setGCPs
 * @param {Object[]} gcps
 * @param {String} projection
 */
NAN_METHOD(Dataset::setGCPs)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		Nan::ThrowError("Dataset does not support setting GCPs");
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	Local<Array> gcps;
	std::string projection("");
	NODE_ARG_ARRAY(0, "gcps", gcps);
	NODE_ARG_OPT_STR(1, "projection", projection);

	GDAL_GCP *list = new GDAL_GCP [gcps->Length()];
	std::string *pszId_list = new std::string [gcps->Length()];
	std::string *pszInfo_list = new std::string [gcps->Length()];
	GDAL_GCP *gcp = list;
	for (unsigned int i = 0; i < gcps->Length(); ++i) {
		Local<Value> val = gcps->Get(i);
		if (!val->IsObject()) {
			if (list) {
				delete [] list;
				delete [] pszId_list;
				delete [] pszInfo_list;
			}
			Nan::ThrowError("GCP array must only include objects");
			return;
		}
		Local<Object> obj = val.As<Object>();

		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPPixel", gcp->dfGCPPixel);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPLine", gcp->dfGCPLine);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPX", gcp->dfGCPX);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPY", gcp->dfGCPY);
		NODE_DOUBLE_FROM_OBJ_OPT(obj, "dfGCPZ", gcp->dfGCPZ);
		NODE_STR_FROM_OBJ_OPT(obj, "pszId", pszId_list[i]);
		NODE_STR_FROM_OBJ_OPT(obj, "pszInfo", pszInfo_list[i]);

		gcp->pszId = (char*) pszId_list[i].c_str();
		gcp->pszInfo = (char*) pszInfo_list[i].c_str();

		gcp++;
	}

	CPLErr err = raw->SetGCPs(gcps->Length(), list, projection.c_str());

	if (list) {
		delete [] list;
		delete [] pszId_list;
		delete [] pszInfo_list;
	}

	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Builds dataset overviews.
 *
 * @throws Error
 * @method buildOverviews
 * @param {String} resampling `"NEAREST"`, `"GAUSS"`, `"CUBIC"`, `"AVERAGE"`, `"MODE"`, `"AVERAGE_MAGPHASE"` or `"NONE"`
 * @param {Integer[]} overviews
 * @param {Integer[]} [bands] Note: Generation of overviews in external TIFF currently only supported when operating on all bands.
 */
NAN_METHOD(Dataset::buildOverviews)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		Nan::ThrowError("Dataset does not support building overviews");
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	std::string resampling = "";
	Local<Array> overviews;
	Local<Array> bands;

	NODE_ARG_STR(0, "resampling", resampling);
	NODE_ARG_ARRAY(1, "overviews", overviews);
	NODE_ARG_ARRAY_OPT(2, "bands", bands);

	int *o, *b = NULL;
	int n_overviews = overviews->Length();
	int i, n_bands = 0;

	o = new int[n_overviews];
	for(i = 0; i<n_overviews; i++){
		Local<Value> val = overviews->Get(i);
		if(!val->IsNumber()) {
			delete [] o;
			Nan::ThrowError("overviews array must only contain numbers");
			return;
		}
		o[i] = val->Int32Value();
	}

	if(!bands.IsEmpty()){
		n_bands = bands->Length();
		b = new int[n_bands];
		for(i = 0; i<n_bands; i++){
			Local<Value> val = bands->Get(i);
			if(!val->IsNumber()) {
				delete [] o;
				delete [] b;
				Nan::ThrowError("band array must only contain numbers");
				return;
			}
			b[i] = val->Int32Value();
			if(b[i] > raw->GetRasterCount() || b[i] < 1) {
				//BuildOverviews prints an error but segfaults before returning
				delete [] o;
				delete [] b;
				Nan::ThrowError("invalid band id");
				return;
			}
		}
	}

	CPLErr err = raw->BuildOverviews(resampling.c_str(), n_overviews, o, n_bands, b, NULL, NULL);

	delete [] o;
	if(b) delete [] b;

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * @readOnly
 * @attribute description
 * @type String
 */
NAN_GETTER(Dataset::descriptionGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		info.GetReturnValue().Set(SafeString::New(raw->GetName()));
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}
	info.GetReturnValue().Set(SafeString::New(raw->GetDescription()));
}

/**
 * Raster dimensions. An object containing `x` and `y` properties.
 *
 * @readOnly
 * @attribute rasterSize
 * @type Object
 */
NAN_GETTER(Dataset::rasterSizeGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();

	//GDAL 2.x will return 512x512 for vector datasets... which doesn't really make sense in JS where we can return null instead of a number
	//https://github.com/OSGeo/gdal/blob/beef45c130cc2778dcc56d85aed1104a9b31f7e6/gdal/gcore/gdaldataset.cpp#L173-L174
	#if GDAL_VERSION_MAJOR >= 2
	if(!raw->GetDriver()->GetMetadataItem(GDAL_DCAP_RASTER)){
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	#endif

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("x").ToLocalChecked(), Nan::New<Integer>(raw->GetRasterXSize()));
	result->Set(Nan::New("y").ToLocalChecked(), Nan::New<Integer>(raw->GetRasterYSize()));
	info.GetReturnValue().Set(result);
}

/**
 * Spatial reference associated with raster dataset
 *
 * @throws Error
 * @attribute srs
 * @type {gdal.SpatialReference}
 */
NAN_GETTER(Dataset::srsGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	//get projection wkt and return null if not set
	char* wkt = (char*) raw->GetProjectionRef();
	if (*wkt == '\0') {
		//getProjectionRef returns string of length 0 if no srs set
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	//otherwise construct and return SpatialReference from wkt
	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromWkt(&wkt);

	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * An affine transform which maps pixel/line coordinates into georeferenced space using the following relationship:
 *
 * @example
 * ```
 * var GT = dataset.geoTransform;
 * var Xgeo = GT[0] + Xpixel*GT[1] + Yline*GT[2];
 * var Ygeo = GT[3] + Xpixel*GT[4] + Yline*GT[5];```
 *
 * @attribute geoTransform
 * @type {Array}
 */
NAN_GETTER(Dataset::geoTransformGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	double transform[6];
	CPLErr err = raw->GetGeoTransform(transform);
	if(err) {
		// This is mostly (always?) a sign that it has not been set
		info.GetReturnValue().Set(Nan::Null());
		return;
		//NODE_THROW_CPLERR(err);
	}

	Local<Array> result = Nan::New<Array>(6);
	result->Set(0, Nan::New<Number>(transform[0]));
	result->Set(1, Nan::New<Number>(transform[1]));
	result->Set(2, Nan::New<Number>(transform[2]));
	result->Set(3, Nan::New<Number>(transform[3]));
	result->Set(4, Nan::New<Number>(transform[4]));
	result->Set(5, Nan::New<Number>(transform[5]));

	info.GetReturnValue().Set(result);
}

/**
 * @readOnly
 * @attribute driver
 * @type {gdal.Driver}
 */
NAN_GETTER(Dataset::driverGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		info.GetReturnValue().Set(Driver::New(raw->GetDriver()));
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	info.GetReturnValue().Set(Driver::New(raw->GetDriver()));
}

NAN_SETTER(Dataset::srsSetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		Nan::ThrowError("Dataset doesnt support setting a spatial reference");
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();
	std::string wkt("");
	if (IS_WRAPPED(value, SpatialReference)) {

		SpatialReference *srs_obj = Nan::ObjectWrap::Unwrap<SpatialReference>(value.As<Object>());
		OGRSpatialReference *srs = srs_obj->get();
		//Get wkt from OGRSpatialReference
		char* str;
		if (srs->exportToWkt(&str)) {
			Nan::ThrowError("Error exporting srs to wkt");
			return;
		}
		wkt = str; //copy string
		CPLFree(str);

	} else if (!value->IsNull() && !value->IsUndefined()) {
		Nan::ThrowError("srs must be SpatialReference object");
		return;
	}

	CPLErr err = raw->SetProjection(wkt.c_str());

	if(err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(Dataset::geoTransformSetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());

	if(!ds->isAlive()){
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR < 2
	if (ds->uses_ogr) {
		Nan::ThrowError("Dataset doesnt support setting a geotransform");
		return;
	}
	#endif

	GDALDataset* raw = ds->getDataset();

	if (!value->IsArray()) {
		Nan::ThrowError("Transform must be an array");
		return;
	}
	Local<Array> transform = value.As<Array>();

	if (transform->Length() != 6) {
		Nan::ThrowError("Transform array must have 6 elements");
		return;
	}

	double buffer[6];
	for (int i = 0; i < 6; i++) {
		Local<Value> val = transform->Get(i);
		if (!val->IsNumber()) {
			Nan::ThrowError("Transform array must only contain numbers");
			return;
		}
		buffer[i] = val->NumberValue();
	}

	CPLErr err = raw->SetGeoTransform(buffer);
	if(err) {
		NODE_THROW_CPLERR(err);
	}
}

/**
 * @readOnly
 * @attribute bands
 * @type {gdal.DatasetBands}
 */
NAN_GETTER(Dataset::bandsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("bands_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute layers
 * @type {gdal.DatasetLayers}
 */
NAN_GETTER(Dataset::layersGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("layers_").ToLocalChecked()).ToLocalChecked());
}

NAN_GETTER(Dataset::uidGetter)
{
	Nan::HandleScope scope;
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(info.This());
	info.GetReturnValue().Set(Nan::New((int)ds->uid));
}

} // namespace node_gdal
