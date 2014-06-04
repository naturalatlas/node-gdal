
#include "gdal_common.hpp"

#include "gdal_majorobject.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_dataset.hpp"

Persistent<FunctionTemplate> RasterBand::constructor;
ObjectCache RasterBand::cache;

void RasterBand::Initialize(Handle<Object> target) {
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(RasterBand::New));
	constructor->Inherit(MajorObject::constructor);
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("RasterBand"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getXSize", getXSize);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getYSize", getYSize);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getBand", getBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getDataset", getDataset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getRasterDataType", getRasterDataType);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getBlockSize", getBlockSize);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getAccess", getAccess);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "rasterIO", rasterIO);
	NODE_SET_PROTOTYPE_METHOD(constructor, "flushCache", flushCache);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getCategoryNames", getCategoryNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNoDataValue", getNoDataValue);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMinimum", getMinimum);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMaximum", getMaximum);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getOffset", getOffset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getScale", getScale);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getUnitType", getUnitType);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getColorTable", getColorTable);
	NODE_SET_PROTOTYPE_METHOD(constructor, "fill", fill);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setCategoryNames", setCategoryNames);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setNoDataValue", setNoDataValue);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "setColorTable", setColorTable);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setOffset", setOffset);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setScale", setScale);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setUnitType", setUnitType);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getStatistics", getStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "computeStatistics", computeStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setStatistics", setStatistics);
	NODE_SET_PROTOTYPE_METHOD(constructor, "hasArbitraryOverviews", hasArbitraryOverviews);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getOverviewCount", getOverviewCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getOverview", getOverview);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getRasterSampleOverview", getRasterSampleOverview);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "buildOverviews", buildOverviews);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getHistogram", getHistogram);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getDefaultHistogram", getDefaultHistogram);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "setDefaultHistogram", setDefaultHistogram);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMaskBand", getMaskBand);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getMaskFlags", getMaskFlags);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createMaskBand", createMaskBand);

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

void RasterBand::dispose(){
	GDALRasterBand *band;
	RasterBand *band_wrapped;
	if(this_) {
		//dispose of all wrapped overview bands
		int n = this_->GetOverviewCount();
		for(int i = 0; i < n; i++) {
			band = this_->GetOverview(i);
			if(RasterBand::cache.has(band)){
				band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
				band_wrapped->dispose();
			}
		}

		//dispose of wrapped mask band
		band = this_->GetMaskBand();
		if(RasterBand::cache.has(band)){
			band_wrapped = ObjectWrap::Unwrap<RasterBand>(RasterBand::cache.get(band));
			band_wrapped->dispose();
		}

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
		return args.This();
	} else {
		return NODE_THROW("Cannot create band directly create with dataset instead");
	}
}

Handle<Value> RasterBand::New(GDALRasterBand *raw) {
	HandleScope scope;

	if(!raw) return Null();
	if(cache.has(raw)) return cache.get(raw);

	RasterBand *wrapped = new RasterBand(raw);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = RasterBand::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	return scope.Close(obj);
}

Handle<Value> RasterBand::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("RasterBand"));
}

NODE_WRAPPED_METHOD(RasterBand, flushCache, FlushCache);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getXSize, Integer, GetXSize);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getYSize, Integer, GetYSize);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getBand, Integer, GetBand);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getDataset, Dataset, GetDataset);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getRasterDataType, Integer, GetRasterDataType);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getAccess, Integer, GetAccess);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getNoDataValue, Number, GetNoDataValue);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMinimum, Number, GetMinimum);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaximum, Number, GetMaximum);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getOffset, Number, GetOffset);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getScale, Number, GetScale);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getUnitType, String, GetUnitType);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, hasArbitraryOverviews, Boolean, HasArbitraryOverviews);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getOverviewCount, Integer, GetOverviewCount);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaskBand, RasterBand, GetMaskBand);
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaskFlags, Integer, GetMaskFlags);
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_DOUBLE_PARAM(RasterBand, setNoDataValue, SetNoDataValue, "no data value");
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_DOUBLE_PARAM(RasterBand, setOffset, SetOffset, "offset");
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_DOUBLE_PARAM(RasterBand, setScale, SetScale, "scale");
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_STRING_PARAM(RasterBand, setUnitType, SetUnitType, "unit type");
NODE_WRAPPED_METHOD_WITH_RESULT_1_INTEGER_PARAM(RasterBand, getOverview, RasterBand, GetOverview, "overview index");
NODE_WRAPPED_METHOD_WITH_RESULT_1_INTEGER_PARAM(RasterBand, getRasterSampleOverview, RasterBand, GetRasterSampleOverview, "number of desired samples");
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_INTEGER_PARAM(RasterBand, createMaskBand, CreateMaskBand, "number of desired samples");


Handle<Value> RasterBand::getBlockSize(const Arguments& args)
{
	HandleScope scope;

	int x, y;
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");
	band->this_->GetBlockSize(&x, &y);

	Local<Object> size = Object::New();
	size->Set(String::NewSymbol("x"), Number::New(x));
	size->Set(String::NewSymbol("y"), Number::New(y));

	return scope.Close(size);
}

Handle<Value> RasterBand::getCategoryNames(const Arguments& args)
{
	HandleScope scope;

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");
	char ** names = band->this_->GetCategoryNames();

	Handle<Array> results = Array::New();

	if(names){
		int i = 0;
		while (names[i]) {
			results->Set(i, String::New(names[i]));
			i++;
		}
	}

	return scope.Close(results);
}

Handle<Value> RasterBand::setCategoryNames(const Arguments& args)
{
	HandleScope scope;

	Handle<Array> names;
	NODE_ARG_ARRAY(0, "category names", names);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");

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

	if (list) delete [] list;

	if(err) return NODE_THROW_CPLERR(err);
	return Undefined();
}

Handle<Value> RasterBand::fill(const Arguments& args)
{
	HandleScope scope;
	double real, imaginary = 0;
	NODE_ARG_DOUBLE(0, "real value", real);
	NODE_ARG_DOUBLE_OPT(1, "imaginary value", real);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");

	int err = band->this_->Fill(real, imaginary);

	if(err) return NODE_THROW_CPLERR(err);
	return Undefined();
}

Handle<Value> RasterBand::getStatistics(const Arguments& args)
{
	HandleScope scope;
	double min, max, mean, std_dev;
	bool approx, force;
	NODE_ARG_BOOL(0, "allow approximation", approx);
	NODE_ARG_BOOL(1, "force", force);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");
	int err = band->this_->GetStatistics(approx, force, &min, &max, &mean, &std_dev);

	if (err) {
		if (!force && err == CE_Warning){
			return NODE_THROW("Statistics cannot be efficiently computed without scanning raster");
		}
		return NODE_THROW("Error getting statistics");
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
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");

	if (band->this_->ComputeStatistics(approx, &min, &max, &mean, &std_dev, NULL, NULL)) {
		return NODE_THROW("Error computing statistics");
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
	if(!band->this_) return NODE_THROW("RasterBand object has already been destroyed");

	int err = band->this_->SetStatistics(min, max, mean, std_dev);

	if(err) return NODE_THROW_CPLERR(err);
	return Undefined();
}