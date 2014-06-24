
#include "gdal_common.hpp"

#include "gdal_majorobject.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_dataset.hpp"
#include "collections/rasterband_overviews.hpp"

#include <limits>
#include <cpl_port.h>

namespace node_gdal {

Persistent<FunctionTemplate> RasterBand::constructor;
ObjectCache<GDALRasterBand*> RasterBand::cache;

void RasterBand::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(RasterBand::New));
	constructor->Inherit(MajorObject::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("RasterBand"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "flush", flush);
	NODE_SET_PROTOTYPE_METHOD(constructor, "fill", fill);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getStatistics", getStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setStatistics", setStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "computeStatistics", computeStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMaskBand", getMaskBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMaskFlags", getMaskFlags);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createMaskBand", createMaskBand);
	
	// unimplemented methods
	//NODE_SET_PROTOTYPE_METHOD(constructor, "buildOverviews", buildOverviews);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "rasterIO", rasterIO);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getColorTable", getColorTable);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "setColorTable", setColorTable);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getHistogram", getHistogram);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getDefaultHistogram", getDefaultHistogram);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "setDefaultHistogram", setDefaultHistogram);

	ATTR_DONT_ENUM(constructor, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "id", idGetter, READ_ONLY_SETTER);
	ATTR(constructor, "size", sizeGetter, READ_ONLY_SETTER);
	ATTR(constructor, "overviews", overviewsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "blockSize", blockSizeGetter, READ_ONLY_SETTER);
	ATTR(constructor, "minimum", minimumGetter, READ_ONLY_SETTER);
	ATTR(constructor, "maximum", maximumGetter, READ_ONLY_SETTER);
	ATTR(constructor, "readOnly", readOnlyGetter, READ_ONLY_SETTER);
	ATTR(constructor, "dataType", dataTypeGetter, READ_ONLY_SETTER);
	ATTR(constructor, "hasArbitraryOverviews", hasArbitraryOverviewsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "unitType", unitTypeGetter, unitTypeSetter);
	ATTR(constructor, "scale", scaleGetter, scaleSetter);
	ATTR(constructor, "offset", offsetGetter, offsetSetter);
	ATTR(constructor, "noDataValue", noDataValueGetter, noDataValueSetter);
	ATTR(constructor, "categoryNames", categoryNamesGetter, categoryNamesSetter);
	ATTR(constructor, "colorInterpretation", colorInterpretationGetter, colorInterpretationSetter);

	target->Set(String::NewSymbol("RasterBand"), constructor->GetFunction());
}

RasterBand::RasterBand(GDALRasterBand *band)
	: ObjectWrap(), this_(band)
{}

RasterBand::RasterBand()
	: ObjectWrap(), this_(0)
{
}

RasterBand::~RasterBand()
{
	dispose();
}

void RasterBand::dispose()
{
	GDALRasterBand *band;
	RasterBand *band_wrapped;

	if (this_) {
		//dispose of all wrapped overview bands
		int n = this_->GetOverviewCount();
		for(int i = 0; i < n; i++) {
			band = this_->GetOverview(i);
			if (RasterBand::cache.has(band)) {
				band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
				band_wrapped->dispose();
			}
		}

		//dispose of wrapped mask band
		band = this_->GetMaskBand();
		if (RasterBand::cache.has(band)) {
			band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
			band_wrapped->dispose();
		}
#ifdef VERBOSE_GC
		printf("Disposing band [%p]\n", this_);
#endif
		this_ = NULL;
	}
}


Handle<Value> RasterBand::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		RasterBand *f = static_cast<RasterBand *>(ptr);
		f->Wrap(args.This());

		Handle<Value> overviews = RasterBandOverviews::New(args.This()); 
		args.This()->SetHiddenValue(String::NewSymbol("overviews_"), overviews); 

		return args.This();
	} else {
		return NODE_THROW("Cannot create band directly create with dataset instead");
	}
}

Handle<Value> RasterBand::New(GDALRasterBand *raw)
{
	HandleScope scope;

	if (!raw) {
		return Null();
	}
	if (cache.has(raw)) {
		return cache.get(raw);
	}

	RasterBand *wrapped = new RasterBand(raw);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = RasterBand::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	//add reference to dataset so dataset doesnt get GC'ed while band is alive
	GDALDataset *parent = raw->GetDataset();
	if (parent) {
		Handle<Value> ds;
		if (Dataset::dataset_cache.has(parent)) {
			ds = Dataset::dataset_cache.get(parent);
		} else {
			ds = Dataset::New(parent); //this should never happen
		}
		obj->SetHiddenValue(String::NewSymbol("ds_"), ds);
	}

	return scope.Close(obj);
}

Handle<Value> RasterBand::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("RasterBand"));
}

NODE_WRAPPED_METHOD(RasterBand, flush, FlushCache);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaskBand, RasterBand, GetMaskBand);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaskFlags, Integer, GetMaskFlags);
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_INTEGER_PARAM(RasterBand, createMaskBand, CreateMaskBand, "number of desired samples");

Handle<Value> RasterBand::fill(const Arguments& args)
{
	HandleScope scope;
	double real, imaginary = 0;
	NODE_ARG_DOUBLE(0, "real value", real);
	NODE_ARG_DOUBLE_OPT(1, "imaginary value", real);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int err = band->this_->Fill(real, imaginary);

	if (err) {
		return NODE_THROW_CPLERR(err);
	}
	return Undefined();
}

// --- Custom error handling to handle VRT errors --- 
// see: https://github.com/mapbox/mapnik-omnivore/issues/10

std::string stats_file_err = "";
CPLErrorHandler last_err_handler;
void statisticsErrorHandler(CPLErr eErrClass, int err_no, const char *msg) {
	if(err_no == CPLE_OpenFailed) {
		stats_file_err = msg;
	}
	if(last_err_handler) {
		last_err_handler(eErrClass, err_no, msg);
	}
}
void pushStatsErrorHandler() {
	last_err_handler = CPLSetErrorHandler(statisticsErrorHandler);
}
void popStatsErrorHandler() {
	if(!last_err_handler) return;
	CPLSetErrorHandler(last_err_handler);
}
	

Handle<Value> RasterBand::getStatistics(const Arguments& args)
{
	HandleScope scope;
	double min, max, mean, std_dev;
	bool approx, force;
	NODE_ARG_BOOL(0, "allow approximation", approx);
	NODE_ARG_BOOL(1, "force", force);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}
	
	pushStatsErrorHandler();
	CPLErr err = band->this_->GetStatistics(approx, force, &min, &max, &mean, &std_dev);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		NODE_THROW(stats_file_err.c_str());
	} else if (err) {
		if (!force && err == CE_Warning) {
			return NODE_THROW("Statistics cannot be efficiently computed without scanning raster");
		}
		return NODE_THROW_CPLERR(err);
	}

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("min"), Number::New(min));
	result->Set(String::NewSymbol("max"), Number::New(max));
	result->Set(String::NewSymbol("mean"), Number::New(mean));
	result->Set(String::NewSymbol("std_dev"), Number::New(std_dev));

	return scope.Close(result);
}

Handle<Value> RasterBand::computeStatistics(const Arguments& args)
{
	HandleScope scope;
	double min, max, mean, std_dev;
	bool approx;
	NODE_ARG_BOOL(0, "allow approximation", approx);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	pushStatsErrorHandler();
	CPLErr err = band->this_->ComputeStatistics(approx, &min, &max, &mean, &std_dev, NULL, NULL);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		NODE_THROW(stats_file_err.c_str());
	} else if (err) {
		return NODE_THROW_CPLERR(err);
	}

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("min"), Number::New(min));
	result->Set(String::NewSymbol("max"), Number::New(max));
	result->Set(String::NewSymbol("mean"), Number::New(mean));
	result->Set(String::NewSymbol("std_dev"), Number::New(std_dev));

	return scope.Close(result);
}

Handle<Value> RasterBand::setStatistics(const Arguments& args)
{
	HandleScope scope;
	double min, max, mean, std_dev;

	NODE_ARG_DOUBLE(0, "min", min);
	NODE_ARG_DOUBLE(1, "max", max);
	NODE_ARG_DOUBLE(2, "mean", mean);
	NODE_ARG_DOUBLE(3, "standard deviation", std_dev);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	CPLErr err = band->this_->SetStatistics(min, max, mean, std_dev);

	if (err) {
		return NODE_THROW_CPLERR(err);
	}
	return Undefined();
}

Handle<Value> RasterBand::dsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("ds_")));
}

Handle<Value> RasterBand::overviewsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("overviews_")));
}

Handle<Value> RasterBand::idGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int id = band->this_->GetBand();

	return id == 0 ? Null() : scope.Close(Integer::New(id));
}

Handle<Value> RasterBand::sizeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("x"), Integer::New(band->this_->GetXSize()));
	result->Set(String::NewSymbol("y"), Integer::New(band->this_->GetYSize()));
	return scope.Close(result);
}

Handle<Value> RasterBand::blockSizeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y;
	band->this_->GetBlockSize(&x, &y);

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("x"), Integer::New(x));
	result->Set(String::NewSymbol("y"), Integer::New(y));
	return scope.Close(result);
}

Handle<Value> RasterBand::minimumGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int success = 0;
	double result = band->this_->GetMinimum(&success);
	return scope.Close(Number::New(result));
}

Handle<Value> RasterBand::maximumGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int success = 0;
	double result = band->this_->GetMaximum(&success);
	return scope.Close(Number::New(result));
}

Handle<Value> RasterBand::offsetGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int success = 0;
	double result = band->this_->GetOffset(&success);
	return scope.Close(Number::New(result));
}

Handle<Value> RasterBand::scaleGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int success = 0;
	double result = band->this_->GetScale(&success);
	return scope.Close(Number::New(result));
}

Handle<Value> RasterBand::noDataValueGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int success = 0;
	double result = band->this_->GetNoDataValue(&success);


	return scope.Close(success && !CPLIsNan(result) ? Number::New(result) : Null());
}

Handle<Value> RasterBand::unitTypeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	const char *result = band->this_->GetUnitType();
	return scope.Close(SafeString::New(result));
}

Handle<Value> RasterBand::dataTypeGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	GDALDataType type = band->this_->GetRasterDataType();
	
	if(type == GDT_Unknown) return Undefined();
	return scope.Close(SafeString::New(GDALGetDataTypeName(type)));
}

Handle<Value> RasterBand::readOnlyGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	GDALAccess result = band->this_->GetAccess();
	return scope.Close(result == GA_Update ? False() : True());
}

Handle<Value> RasterBand::hasArbitraryOverviewsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	bool result = band->this_->HasArbitraryOverviews();
	return scope.Close(Boolean::New(result));
}

Handle<Value> RasterBand::categoryNamesGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	char ** names = band->this_->GetCategoryNames();

	Handle<Array> results = Array::New();

	if (names) {
		int i = 0;
		while (names[i]) {
			results->Set(i, String::New(names[i]));
			i++;
		}
	}

	return scope.Close(results);
}

Handle<Value> RasterBand::colorInterpretationGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}
	GDALColorInterp interp = band->this_->GetColorInterpretation();
	if(interp == GCI_Undefined) return Undefined();
	else return scope.Close(SafeString::New(GDALGetColorInterpretationName(interp)));
}

void RasterBand::unitTypeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsString()) {
		NODE_THROW("Unit type must be a string");
		return;
	}
	const char *input = TOSTR(value);
	CPLErr err = band->this_->SetUnitType(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void RasterBand::noDataValueSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed"); 
		return;
	}
	
	double input;
	
	if (value->IsNull() || value -> IsUndefined()){
		input = std::numeric_limits<double>::quiet_NaN();
	} else if (value->IsNumber()) {
		input = value->NumberValue();
	} else {
		NODE_THROW("No data value must be a number");
		return;
	}

	CPLErr err = band->this_->SetNoDataValue(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void RasterBand::scaleSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsNumber()) {
		NODE_THROW("Scale must be a number");
		return;
	}
	double input = value->NumberValue();
	CPLErr err = band->this_->SetScale(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void RasterBand::offsetSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed"); 
		return;
	}

	if (!value->IsNumber()) {
		NODE_THROW("Offset must be a number");
		return;
	}
	double input = value->NumberValue();
	CPLErr err = band->this_->SetOffset(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void RasterBand::categoryNamesSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed"); 
		return;
	}

	if(!value->IsArray()){
		NODE_THROW("Category names must be an array"); 
		return;
	}
	Handle<Array> names = Handle<Array>::Cast(value);

	char **list = NULL;
	if (names->Length() > 0) {
		list = new char* [names->Length() + 1];
		unsigned int i;
		for (i = 0; i < names->Length(); i++) {
			list[i] = TOSTR(names->Get(i));
		}
		list[i] = NULL;
	}

	int err = band->this_->SetCategoryNames(list);

	if (list) {
		delete [] list;
	}

	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

void RasterBand::colorInterpretationSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->this_) {
		NODE_THROW("RasterBand object has already been destroyed");
		return;
	}

	GDALColorInterp ci = GCI_Undefined;
	
	if (value->IsString()) {
		ci = GDALGetColorInterpretationByName(TOSTR(value));
	} else if(!value->IsNull() && !value->IsUndefined()) {
		NODE_THROW("color interpretation must be a string or undefined");
		return;
	}

	CPLErr err = band->this_->SetColorInterpretation(ci);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

} // namespace node_gdal