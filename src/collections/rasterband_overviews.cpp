#include "../gdal_common.hpp"
#include "../gdal_rasterband.hpp"
#include "rasterband_overviews.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> RasterBandOverviews::constructor;

void RasterBandOverviews::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(RasterBandOverviews::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("RasterBandOverviews"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getBySampleCount", getBySampleCount);

	target->Set(NanNew("RasterBandOverviews"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

RasterBandOverviews::RasterBandOverviews()
	: ObjectWrap()
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
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		RasterBandOverviews *f =  static_cast<RasterBandOverviews *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create RasterBandOverviews directly");
		NanReturnUndefined();
	}
}

Handle<Value> RasterBandOverviews::New(Handle<Value> band_obj)
{
	NanEscapableScope();

	RasterBandOverviews *wrapped = new RasterBandOverviews();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(RasterBandOverviews::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), band_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(RasterBandOverviews::toString)
{
	NanScope();
	NanReturnValue(NanNew("RasterBandOverviews"));
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int id;
	NODE_ARG_INT(0, "id", id);

	GDALRasterBand *result = band->get()->GetOverview(id);

	// TODO: return null instead?
	if (result == NULL) {
		NanThrowError("Specified overview not found");
		NanReturnUndefined();
	}

	NanReturnValue(RasterBand::New(result, band->getParent()));
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
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int n_samples;
	NODE_ARG_INT(0, "minimum number of samples", n_samples);

	GDALRasterBand *result = band->get()->GetRasterSampleOverview(n_samples);

	NanReturnValue(RasterBand::New(result, band->getParent()));
}

/**
 * Returns the number of overviews.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(RasterBandOverviews::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(NanNew<Integer>(band->get()->GetOverviewCount()));
}

} // namespace node_gdal