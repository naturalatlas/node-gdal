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

	if (result == NULL) {
		NanThrowError("Specified overview not found");
		NanReturnUndefined();
	}

	NanReturnValue(RasterBand::New(result, band->getParent()));
}

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