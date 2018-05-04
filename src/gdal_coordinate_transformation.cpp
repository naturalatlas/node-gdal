
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_coordinate_transformation.hpp"
#include "gdal_dataset.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> CoordinateTransformation::constructor;

void CoordinateTransformation::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(CoordinateTransformation::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("CoordinateTransformation").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "transformPoint", transformPoint);

	target->Set(Nan::New("CoordinateTransformation").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

CoordinateTransformation::CoordinateTransformation(OGRCoordinateTransformation *transform)
	: Nan::ObjectWrap(),
	  this_(transform)
{
	LOG("Created CoordinateTransformation [%p]", transform);
}

CoordinateTransformation::CoordinateTransformation()
	: Nan::ObjectWrap(),
	  this_(0)
{
}

CoordinateTransformation::~CoordinateTransformation()
{
	if (this_) {
		LOG("Disposing CoordinateTransformation [%p]", this_);
		OGRCoordinateTransformation::DestroyCT(this_);
		LOG("Disposed CoordinateTransformation [%p]", this_);
		this_ = NULL;
	}
}

/**
 * Object for transforming between coordinate systems.
 *
 * @throws Error
 * @constructor
 * @class gdal.CoordinateTransformation
 * @param {gdal.SpatialReference} source
 * @param {gdal.SpatialReference|gdal.Dataset} target If a raster Dataset, the conversion will represent a conversion to pixel coordinates.
 */
NAN_METHOD(CoordinateTransformation::New)
{
	Nan::HandleScope scope;
	CoordinateTransformation *f;
	SpatialReference *source, *target;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f =  static_cast<CoordinateTransformation *>(ptr);
	} else {
		if(info.Length() < 2) {
			Nan::ThrowError("Invalid number of arguments");
			return;
		}

		NODE_ARG_WRAPPED(0, "source", SpatialReference, source);

		if(!info[1]->IsObject() || info[1]->IsNull()){
			Nan::ThrowTypeError("target must be a SpatialReference or Dataset object");
			return;
		}
		if(Nan::New(SpatialReference::constructor)->HasInstance(info[1])) {
			// srs -> srs
			NODE_ARG_WRAPPED(1, "target", SpatialReference, target);

			OGRCoordinateTransformation * transform = OGRCreateCoordinateTransformation(source->get(), target->get());
			if (!transform) {
				NODE_THROW_LAST_CPLERR();
				return;
			}
			f = new CoordinateTransformation(transform);
		} else if(Nan::New(Dataset::constructor)->HasInstance(info[1])) {
			// srs -> px/line
			// todo: allow additional options using StringList

			Dataset *ds;
			char** papszTO = NULL;
			char* src_wkt;

			ds = Nan::ObjectWrap::Unwrap<Dataset>(info[1].As<Object>());

			if(!ds->getDataset()){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()){
					Nan::ThrowError("Only raster datasets can be used to create geotransform coordinate transformations");
					return;
				}
				#endif
				Nan::ThrowError("Dataset already closed");
				return;
			}

			OGRErr err = source->get()->exportToWkt(&src_wkt);
			if(err) {
				NODE_THROW_OGRERR(err);
				return;
			}

			papszTO = CSLSetNameValue( papszTO, "DST_SRS", src_wkt );
			papszTO = CSLSetNameValue( papszTO, "INSERT_CENTER_LONG", "FALSE" );

			GeoTransformTransformer* transform = new GeoTransformTransformer();
			transform->hSrcImageTransformer = GDALCreateGenImgProjTransformer2( ds->getDataset(), NULL, papszTO );
			if(!transform->hSrcImageTransformer){
				NODE_THROW_LAST_CPLERR();
				return;
			}

			f = new CoordinateTransformation(transform);

			CPLFree(src_wkt);
			CSLDestroy(papszTO);
		} else {
			Nan::ThrowTypeError("target must be a SpatialReference or Dataset object");
			return;
		}
	}

	f->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

Local<Value> CoordinateTransformation::New(OGRCoordinateTransformation *transform)
{
	Nan::EscapableHandleScope scope;

	if (!transform) {
		return scope.Escape(Nan::Null());
	}

	CoordinateTransformation *wrapped = new CoordinateTransformation(transform);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(CoordinateTransformation::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(CoordinateTransformation::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("CoordinateTransformation").ToLocalChecked());
}

/**
 * Transform point from source to destination space.
 *
 * @example
 * ```
 * pt = transform.transformPoint(0, 0, 0);
 * pt = transform.transformPoint({x: 0, y: 0, z: 0});```
 *
 * @method transformPoint
 * @param {Number} x
 * @param {Number} y
 * @param {Number} [z]
 * @return {Object} A regular object containing `x`, `y`, `z` properties.
 */
NAN_METHOD(CoordinateTransformation::transformPoint)
{
	Nan::HandleScope scope;

	CoordinateTransformation *transform = Nan::ObjectWrap::Unwrap<CoordinateTransformation>(info.This());

	double x, y, z = 0;

	if (info.Length() == 1 && info[0]->IsObject()) {
		Local<Object> obj = info[0].As<Object>();
		Local<Value> arg_x = obj->Get(Nan::New("x").ToLocalChecked());
		Local<Value> arg_y = obj->Get(Nan::New("y").ToLocalChecked());
		Local<Value> arg_z = obj->Get(Nan::New("z").ToLocalChecked());
		if (!arg_x->IsNumber() || !arg_y->IsNumber()) {
			Nan::ThrowError("point must contain numerical properties x and y");
			return;
		}
		x = static_cast<double>(arg_x->NumberValue());
		y = static_cast<double>(arg_y->NumberValue());
		if (arg_z->IsNumber()) {
			z = static_cast<double>(arg_z->NumberValue());
		}
	} else {
		NODE_ARG_DOUBLE(0, "x", x);
		NODE_ARG_DOUBLE(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);
	}

	if (!transform->this_->Transform(1, &x, &y, &z)) {
		Nan::ThrowError("Error transforming point");
		return;
	}

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("x").ToLocalChecked(), Nan::New<Number>(x));
	result->Set(Nan::New("y").ToLocalChecked(), Nan::New<Number>(y));
	result->Set(Nan::New("z").ToLocalChecked(), Nan::New<Number>(z));

	info.GetReturnValue().Set(result);
}

} // namespace node_gdal