#include "../gdal_common.hpp"
#include "../gdal_rasterband.hpp"
#include "rasterband_overviews.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> RasterBandOverviews::constructor;

void RasterBandOverviews::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(RasterBandOverviews::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("RasterBandOverviews").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "getBySampleCount", getBySampleCount);

	target->Set(Nan::New("RasterBandOverviews").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

RasterBandOverviews::RasterBandOverviews()
	: Nan::ObjectWrap()
{}

RasterBandOverviews::~RasterBandOverviews()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.RasterBand"}}RasterBand{{/crossLink}}
 * overview functionality.
 *
 * @class gdal.RasterBandOverviews
 */
NAN_METHOD(RasterBandOverviews::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		RasterBandOverviews *f =  static_cast<RasterBandOverviews *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create RasterBandOverviews directly");
		return;
	}
}

Local<Value> RasterBandOverviews::New(Local<Value> band_obj)
{
	Nan::EscapableHandleScope scope;

	RasterBandOverviews *wrapped = new RasterBandOverviews();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(RasterBandOverviews::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), band_obj);

	return scope.Escape(obj);
}

NAN_METHOD(RasterBandOverviews::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("RasterBandOverviews").ToLocalChecked());
}

/**
 * Fetches the overview at the provided index.
 *
 * @method get
 * @throws Error
 * @param {Integer} index 0-based index
 * @return {gdal.RasterBand}
 */
NAN_METHOD(RasterBandOverviews::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int id;
	NODE_ARG_INT(0, "id", id);

	GDALRasterBand *result = band->get()->GetOverview(id);

	// TODO: return null instead?
	if (result == NULL) {
		Nan::ThrowError("Specified overview not found");
		return;
	}

	info.GetReturnValue().Set(RasterBand::New(result, band->getParent()));
}

/**
 * Fetch best sampling overview.
 *
 * Returns the most reduced overview of the given band that still satisfies the
 * desired number of samples. This function can be used with zero as the number
 * of desired samples to fetch the most reduced overview. The same band as was
 * passed in will be returned if it has not overviews, or if none of the overviews
 * have enough samples.
 *
 * @method getBySampleCount
 * @param {Integer} samples
 * @return {gdal.RasterBand}
 */
NAN_METHOD(RasterBandOverviews::getBySampleCount)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int n_samples;
	NODE_ARG_INT(0, "minimum number of samples", n_samples);

	GDALRasterBand *result = band->get()->GetRasterSampleOverview(n_samples);

	info.GetReturnValue().Set(RasterBand::New(result, band->getParent()));
}

/**
 * Returns the number of overviews.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(RasterBandOverviews::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(Nan::New<Integer>(band->get()->GetOverviewCount()));
}

} // namespace node_gdal