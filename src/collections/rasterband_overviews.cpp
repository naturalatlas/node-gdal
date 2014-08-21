#include "../gdal_common.hpp"
#include "../gdal_rasterband.hpp"
#include "rasterband_overviews.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> RasterBandOverviews::constructor;

void RasterBandOverviews::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(RasterBandOverviews::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("RasterBandOverviews"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getBySampleCount", getBySampleCount);

	target->Set(String::NewSymbol("RasterBandOverviews"), constructor->GetFunction());
}

RasterBandOverviews::RasterBandOverviews()
	: ObjectWrap()
{}

RasterBandOverviews::~RasterBandOverviews() 
{}

Handle<Value> RasterBandOverviews::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		RasterBandOverviews *f =  static_cast<RasterBandOverviews *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create RasterBandOverviews directly");
	}
}

Handle<Value> RasterBandOverviews::New(Handle<Value> band_obj)
{
	HandleScope scope;

	RasterBandOverviews *wrapped = new RasterBandOverviews();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = RasterBandOverviews::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), band_obj);

	return scope.Close(obj);
}

Handle<Value> RasterBandOverviews::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("RasterBandOverviews"));
}

Handle<Value> RasterBandOverviews::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int id;
	NODE_ARG_INT(0, "id", id);
	
	GDALRasterBand *result = band->get()->GetOverview(id);

	if (result == NULL) {
		return NODE_THROW("Specified overview not found");
	}

	return scope.Close(RasterBand::New(result, band->getParent()));
}

Handle<Value> RasterBandOverviews::getBySampleCount(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int n_samples;
	NODE_ARG_INT(0, "minimum number of samples", n_samples);
	
	GDALRasterBand *result = band->get()->GetRasterSampleOverview(n_samples);

	return scope.Close(RasterBand::New(result, band->getParent()));
}

Handle<Value> RasterBandOverviews::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}
	
	return scope.Close(Integer::New(band->get()->GetOverviewCount()));
}

} // namespace node_gdal