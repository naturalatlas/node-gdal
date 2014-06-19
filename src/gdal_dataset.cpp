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

Persistent<FunctionTemplate> Dataset::constructor;
ObjectCache<GDALDataset*> Dataset::dataset_cache;
ObjectCache<OGRDataSource*> Dataset::datasource_cache;

void Dataset::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Dataset::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Dataset"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGCPs", setGCPs);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGCPs", getGCPs);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGCPProjection", getGCPProjection);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFileList", getFileList);
	NODE_SET_PROTOTYPE_METHOD(constructor, "flush", flush);
	NODE_SET_PROTOTYPE_METHOD(constructor, "close", close);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMetadata", getMetadata);
	NODE_SET_PROTOTYPE_METHOD(constructor, "testCapability", testCapability);
	NODE_SET_PROTOTYPE_METHOD(constructor, "executeSQL", executeSQL);

	ATTR(constructor, "description", descriptionGetter, READ_ONLY_SETTER);
	ATTR(constructor, "bands", bandsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "layers", layersGetter, READ_ONLY_SETTER);
	ATTR(constructor, "rasterSize", rasterSizeGetter, READ_ONLY_SETTER);
	ATTR(constructor, "driver", driverGetter, READ_ONLY_SETTER);
	ATTR(constructor, "srs", srsGetter, srsSetter);
	ATTR(constructor, "geoTransform", geoTransformGetter, geoTransformSetter);

	target->Set(String::NewSymbol("Dataset"), constructor->GetFunction());
}

Dataset::Dataset(GDALDataset *ds)
	: ObjectWrap(),
	  uses_ogr(false),
	  this_dataset(ds),
	  this_datasource(0),
	  result_sets()
{}

Dataset::Dataset(OGRDataSource *ds)
	: ObjectWrap(),
	  uses_ogr(true),
	  this_dataset(0),
	  this_datasource(ds),
	  result_sets()
{}

Dataset::~Dataset()
{
	//Destroy at garbage collection time if not already explicitly destroyed
	dispose();
}

void Dataset::dispose()
{
	GDALRasterBand *band;	
	OGRLayer *lyr;
	Layer *lyr_wrapped;
	
	if (this_dataset) {
		//dispose of all wrapped child bands
		int n = this_dataset->GetRasterCount();
		for(int i = 1; i <= n; i++) {
			band = this_dataset->GetRasterBand(i);
			if (RasterBand::cache.has(band)) {
				RasterBand *band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
				band_wrapped->dispose();
			}
		}
#ifdef VERBOSE_GC
		printf("Disposing dataset [%p]\n", this_dataset);
#endif
		GDALClose(this_dataset);
		this_dataset = NULL;
	} 
	if (this_datasource) {
		//dispose of all wrapped child layers
		int n = this_datasource->GetLayerCount();
		for(int i = 0; i < n; i++) {
			lyr = this_datasource->GetLayer(i);
			if (Layer::cache.has(lyr)) {
				lyr_wrapped = ObjectWrap::Unwrap<Layer>(Layer::cache.get(lyr));
				lyr_wrapped->dispose();
			}
		}

		//dispose of all result sets
		n = result_sets.size();
		for(int i = 0; i < n; i++) {
			lyr = result_sets[i];
			if (Layer::cache.has(lyr)) {
				lyr_wrapped = ObjectWrap::Unwrap<Layer>(Layer::cache.get(lyr));
				lyr_wrapped->dispose();
			}
		}
		result_sets.clear();

#ifdef VERBOSE_GC
		printf("Disposing datasource [%p]\n", this_datasource);
#endif
		OGRDataSource::DestroyDataSource(this_datasource);
		this_datasource = NULL;
	}
}

Handle<Value> Dataset::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		Dataset *f =  static_cast<Dataset *>(ptr);
		f->Wrap(args.This());

		Handle<Value> bands = DatasetBands::New(args.This()); 
		args.This()->SetHiddenValue(String::NewSymbol("bands_"), bands); 

		Handle<Value> layers = DatasetLayers::New(args.This()); 
		args.This()->SetHiddenValue(String::NewSymbol("layers_"), layers); 
		
		return args.This();
	} else {
		return NODE_THROW("Cannot create dataset directly");
	}
}

Handle<Value> Dataset::New(GDALDataset *raw)
{
	HandleScope scope;

	if (!raw) {
		return Null();
	}
	if (dataset_cache.has(raw)) {
		return dataset_cache.get(raw);
	}

	Dataset *wrapped = new Dataset(raw);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Dataset::constructor->GetFunction()->NewInstance(1, &ext);

	dataset_cache.add(raw, obj);

	return scope.Close(obj);
}

Handle<Value> Dataset::New(OGRDataSource *raw)
{
	HandleScope scope;

	if (!raw) {
		return Null();
	}
	if (datasource_cache.has(raw)) {
		return datasource_cache.get(raw);
	}

	Dataset *wrapped = new Dataset(raw);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Dataset::constructor->GetFunction()->NewInstance(1, &ext);

	datasource_cache.add(raw, obj);

	return scope.Close(obj);
}

Handle<Value> Dataset::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Dataset"));
}

Handle<Value> Dataset::getMetadata(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return scope.Close(Object::New());
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}

		std::string domain("");
		NODE_ARG_OPT_STR(0, "domain", domain);
		return scope.Close(MajorObject::getMetadata(raw, domain.empty() ? NULL : domain.c_str()));
	}
}

Handle<Value> Dataset::testCapability(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr && raw) {
			return scope.Close(False());
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	std::string capability("");
	NODE_ARG_STR(0, "capability", capability);

	return scope.Close(Boolean::New(raw->TestCapability(capability.c_str())));
}


Handle<Value> Dataset::getGCPProjection(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
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
		return scope.Close(SafeString::New(raw->GetGCPProjection()));
	}
}

Handle<Value> Dataset::close(const Arguments& args)
{
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->getDataset() && !ds->getDatasource()) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	ds->dispose();

	return Undefined();
}

Handle<Value> Dataset::flush(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
	if (ds->uses_ogr){
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		OGRErr err = raw->SyncToDisk();
		if(err) {
			return NODE_THROW_OGRERR(err);
		}
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		raw->FlushCache();
	}

	return Undefined();
}

Handle<Value> Dataset::executeSQL(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());

	#if GDAL_MAJOR > 2
		GDALDataset* raw = ds->getDataset();
	#else
		OGRDataSource* raw = ds->getDatasource();
		if (!ds->uses_ogr){
			return NODE_THROW("Dataset does not support executing a SQL query")
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

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
		ds->result_sets.push_back(layer);
		return scope.Close(Layer::New(layer, ds, true));
	} else {
		return NODE_THROW("Error executing SQL");
	}
}

Handle<Value> Dataset::getFileList(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());

	Handle<Array> results = Array::New(0);

	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}

		char **list = raw->GetFileList();
		if (!list) {
			return scope.Close(results);
		}

		int i = 0;
		while (list[i]) {
			results->Set(i, String::New(list[i]));
			i++;
		}

		CSLDestroy(list);
	}

	return scope.Close(results);
}

Handle<Value> Dataset::getGCPs(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
	Handle<Array> results = Array::New(0);
	
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}

		int n = raw->GetGCPCount();
		const GDAL_GCP *gcps = raw->GetGCPs();

		if (!gcps) {
			return scope.Close(results);
		}

		for (int i = 0; i < n; i++) {
			GDAL_GCP gcp = gcps[i];
			Local<Object> obj = Object::New();
			obj->Set(String::NewSymbol("pszId"), String::New(gcp.pszId));
			obj->Set(String::NewSymbol("pszInfo"), String::New(gcp.pszInfo));
			obj->Set(String::NewSymbol("dfGCPPixel"), Number::New(gcp.dfGCPPixel));
			obj->Set(String::NewSymbol("dfGCPLine"), Number::New(gcp.dfGCPLine));
			obj->Set(String::NewSymbol("dfGCPX"), Number::New(gcp.dfGCPX));
			obj->Set(String::NewSymbol("dfGCPY"), Number::New(gcp.dfGCPY));
			obj->Set(String::NewSymbol("dfGCPZ"), Number::New(gcp.dfGCPZ));
			results->Set(i, obj);
		}
	}

	return scope.Close(results);
}

Handle<Value> Dataset::setGCPs(const Arguments& args)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	
	if (ds->uses_ogr) {
		return NODE_THROW("Dataset does not support setting GCPs")
	}

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	Handle<Array> gcps;
	std::string projection("");
	NODE_ARG_ARRAY(0, "gcps", gcps);
	NODE_ARG_OPT_STR(1, "projection", projection);

	GDAL_GCP* list = new GDAL_GCP [gcps->Length()];
	GDAL_GCP* gcp = list;
	for (unsigned int i = 0; i < gcps->Length(); ++i) {
		Local<Value> val = gcps->Get(i);
		if (!val->IsObject()) {
			return NODE_THROW("list of GCPs must only contain objects");
		}
		Local<Object> obj = val->ToObject();

		NODE_STR_FROM_OBJ_OPT(obj, "pszId", gcp->pszId);
		NODE_STR_FROM_OBJ_OPT(obj, "pszInfo", gcp->pszInfo);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPPixel", gcp->dfGCPPixel);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPLine", gcp->dfGCPLine);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPX", gcp->dfGCPX);
		NODE_DOUBLE_FROM_OBJ(obj, "dfGCPY", gcp->dfGCPY);
		NODE_DOUBLE_FROM_OBJ_OPT(obj, "dfGCPZ", gcp->dfGCPZ);
		gcp++;
	}

	if (list) {
		delete [] list;
	}

	CPLErr err = raw->SetGCPs(gcps->Length(), list, projection.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Dataset::descriptionGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return SafeString::New(raw->GetName());
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return SafeString::New(raw->GetDescription());
	}
}

Handle<Value> Dataset::rasterSizeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
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
		Local<Object> result = Object::New();
		result->Set(String::NewSymbol("x"), Integer::New(raw->GetRasterXSize()));
		result->Set(String::NewSymbol("y"), Integer::New(raw->GetRasterYSize()));
		return scope.Close(result);
	}
}

Handle<Value> Dataset::srsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
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
		//get projection wkt and return null if not set
		char* wkt = (char*) raw->GetProjectionRef();
		if (*wkt == '\0') {
			//getProjectionRef returns string of length 0 if no srs set
			return Null();
		}
		//otherwise construct and return SpatialReference from wkt
		OGRSpatialReference *srs = new OGRSpatialReference();
		int err = srs->importFromWkt(&wkt);

		if(err) {
			return NODE_THROW_OGRERR(err);
		}

		return scope.Close(SpatialReference::New(srs, true));
	}
	
}

Handle<Value> Dataset::geoTransformGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
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
		double transform[6];
		CPLErr err = raw->GetGeoTransform(transform);
		if (err) {
			return NODE_THROW_CPLERR(err);
		}

		Handle<Array> result = Array::New(6);
		result->Set(0, Number::New(transform[0]));
		result->Set(1, Number::New(transform[1]));
		result->Set(2, Number::New(transform[2]));
		result->Set(3, Number::New(transform[3]));
		result->Set(4, Number::New(transform[4]));
		result->Set(5, Number::New(transform[5]));

		return scope.Close(result);
	}
}

Handle<Value> Dataset::driverGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
		OGRDataSource* raw = ds->getDatasource();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return scope.Close(Driver::New(raw->GetDriver()));
	} else {
		GDALDataset* raw = ds->getDataset();
		if (!raw) {
			return NODE_THROW("Dataset object has already been destroyed");
		}
		return scope.Close(Driver::New(raw->GetDriver()));
	}
}

void Dataset::srsSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	
	if (ds->uses_ogr) {
		NODE_THROW("Dataset doesnt support setting a spatial reference");
		return;
	}

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		NODE_THROW("Dataset object has already been destroyed");
		return;
	}

	std::string wkt("");
	if (SpatialReference::constructor->HasInstance(value)) {
		
		SpatialReference *srs_obj = ObjectWrap::Unwrap<SpatialReference>(value->ToObject());
		OGRSpatialReference *srs = srs_obj->get();
		//Get wkt from OGRSpatialReference
		char* str;
		if (srs->exportToWkt(&str)) {
			NODE_THROW("Error exporting srs to wkt");
			return;
		}
		wkt = str; //copy string
		CPLFree(str);

	} else if (!value->IsNull() && !value->IsUndefined()) {
		NODE_THROW("srs must be SpatialReference object");
		return;
	}

	CPLErr err = raw->SetProjection(wkt.c_str());
	
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void Dataset::geoTransformSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	if (ds->uses_ogr) {
		NODE_THROW("Dataset doesnt support setting a geotransform");
		return;
	}

	GDALDataset* raw = ds->getDataset();
	if (!raw) {
		NODE_THROW("Dataset object has already been destroyed");
		return;
	}

	if (!value->IsArray()) {
		NODE_THROW("Transform must be an array");
		return;
	}
	Handle<Array> transform = Handle<Array>::Cast(value);

	if (transform->Length() != 6) {
		NODE_THROW("Transform array must have 6 elements");
		return;
	}

	double buffer[6];
	for (int i = 0; i < 6; i++) {
		Local<Value> val = transform->Get(i);
		if (!val->IsNumber()) {
			NODE_THROW("Transform array must only contain numbers");
			return;
		}
		buffer[i] = val->NumberValue();
	}

	CPLErr err = raw->SetGeoTransform(buffer);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

Handle<Value> Dataset::bandsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("bands_")));
}

Handle<Value> Dataset::layersGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("layers_")));
}