#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_driver.hpp"
#include "ogr_common.hpp"
#include "ogr_spatial_reference.hpp"

Persistent<FunctionTemplate> Dataset::constructor;
ObjectCache<GDALDataset*> Dataset::cache;

void Dataset::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Dataset::New));
	constructor->Inherit(MajorObject::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Dataset"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getRasterCount", getRasterCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getRasterBand", getRasterBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "addBand", addBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getDriver", getDriver);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGCPCount", getGCPCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGCPProjection", getGCPProjection);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createMaskBand", createMaskBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeoTransform", getGeoTransform);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeoTransform", setGeoTransform);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGCPs", getGCPs);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGCPs", setGCPs);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFileList", getFileList);
	NODE_SET_PROTOTYPE_METHOD(constructor, "flushCache", flushCache);
	NODE_SET_PROTOTYPE_METHOD(constructor, "close", close);

	ATTR(constructor, "size", sizeGetter, READ_ONLY_SETTER);
	ATTR(constructor, "srs", srsGetter, srsSetter);

	target->Set(String::NewSymbol("Dataset"), constructor->GetFunction());
}

Dataset::Dataset(GDALDataset *ds)
	: ObjectWrap(),
	  this_(ds)
{}

Dataset::Dataset()
	: ObjectWrap(),
	  this_(0)
{
}

Dataset::~Dataset()
{
	//Destroy at garbage collection time if not already explicitly destroyed
	dispose();
}

void Dataset::dispose()
{
	GDALRasterBand *band;
	if (this_) {
		//dispose of all wrapped child bands
		int n = this_->GetRasterCount();
		for(int i = 1; i <= n; i++) {
			band = this_->GetRasterBand(i);
			if (RasterBand::cache.has(band)) {
				RasterBand *band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
				band_wrapped->dispose();
			}
		}
#ifdef VERBOSE_GC
		printf("Disposing dataset [%p]\n", this_);
#endif
		GDALClose(this_);
		this_ = NULL;
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
	if (cache.has(raw)) {
		return cache.get(raw);
	}

	Dataset *wrapped = new Dataset(raw);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Dataset::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	return scope.Close(obj);
}

Handle<Value> Dataset::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Dataset"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(Dataset, getRasterCount, Integer, GetRasterCount);
NODE_WRAPPED_METHOD_WITH_RESULT(Dataset, getDriver, Driver, GetDriver);
NODE_WRAPPED_METHOD_WITH_RESULT(Dataset, getGCPCount, Integer, GetGCPCount);
NODE_WRAPPED_METHOD_WITH_RESULT(Dataset, getGCPProjection, SafeString, GetGCPProjection);
NODE_WRAPPED_METHOD_WITH_RESULT_1_INTEGER_PARAM(Dataset, createMaskBand, Integer, CreateMaskBand, "flags");
NODE_WRAPPED_METHOD(Dataset, flushCache, FlushCache);

Handle<Value> Dataset::close(const Arguments& args)
{
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	ds->dispose();

	return Undefined();
}

Handle<Value> Dataset::getRasterBand(const Arguments& args)
{
	HandleScope scope;
	GDALRasterBand *poBand;
	int band_id;
	NODE_ARG_INT(0, "band id", band_id);

	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}
	poBand = ds->this_->GetRasterBand(band_id);

	if (poBand == NULL) {
		return NODE_THROW("Specified band not found");
	}

	return scope.Close(RasterBand::New(poBand));
}

Handle<Value> Dataset::addBand(const Arguments& args)
{
	HandleScope scope;

	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
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

	CPLErr err = ds->this_->AddBand(type, options);

	if (options) {
		delete [] options;
	}

	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return scope.Close(RasterBand::New(ds->this_->GetRasterBand(ds->this_->GetRasterCount())));
}

Handle<Value> Dataset::getGeoTransform(const Arguments& args)
{
	HandleScope scope;

	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	double transform[6];
	CPLErr err = ds->this_->GetGeoTransform(transform);
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

Handle<Value> Dataset::setGeoTransform(const Arguments& args)
{
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	Handle<Array> transform;
	NODE_ARG_ARRAY(0, "transform", transform);

	if (transform->Length() != 6) {
		return NODE_THROW("Transform array must have 6 elements")
	}

		   double buffer[6];
	for (int i = 0; i < 6; i++) {
		Local<Value> val = transform->Get(i);
		if (!val->IsNumber()) {
			return NODE_THROW("Transform array must only contain numbers");
		}
		buffer[i] = val->NumberValue();
	}

	CPLErr err = ds->this_->SetGeoTransform(buffer);
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Dataset::getFileList(const Arguments& args)
{
	HandleScope scope;

	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	char **list = ds->this_->GetFileList();
	Handle<Array> results = Array::New(0);

	if (!list) {
		return scope.Close(results);
	}

	int i = 0;
	while (list[i]) {
		results->Set(i, String::New(list[i]));
		i++;
	}

	CSLDestroy(list);

	return scope.Close(results);
}

Handle<Value> Dataset::getGCPs(const Arguments& args)
{
	HandleScope scope;

	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	int n = ds->this_->GetGCPCount();
	const GDAL_GCP *gcps = ds->this_->GetGCPs();

	Handle<Array> results = Array::New(0);

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

	return scope.Close(results);
}

Handle<Value> Dataset::setGCPs(const Arguments& args)
{
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(args.This());
	if (!ds->this_) {
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

	CPLErr err = ds->this_->SetGCPs(gcps->Length(), list, projection.c_str());
	if (err) {
		return NODE_THROW_CPLERR(err);
	}

	return Undefined();
}

Handle<Value> Dataset::sizeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("x"), Integer::New(ds->this_->GetRasterXSize()));
	result->Set(String::NewSymbol("y"), Integer::New(ds->this_->GetRasterYSize()));
	return scope.Close(result);
}

Handle<Value> Dataset::srsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	if (!ds->this_) {
		return NODE_THROW("Dataset object has already been destroyed");
	}

	//get projection wkt and return null if not set
	char* wkt = (char*) ds->this_->GetProjectionRef();
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

	return scope.Close(node_ogr::SpatialReference::New(srs, true));
}


void Dataset::srsSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(info.This());
	if (!ds->this_) {
		NODE_THROW("Dataset object has already been destroyed");
		return;
	}

	std::string wkt("");
	if (node_ogr::SpatialReference::constructor->HasInstance(value)) {
		
		node_ogr::SpatialReference *srs_obj = ObjectWrap::Unwrap<node_ogr::SpatialReference>(value->ToObject());
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

	CPLErr err = ds->this_->SetProjection(wkt.c_str());
	
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}