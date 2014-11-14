
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_coordinate_transformation.hpp"
#include "gdal_dataset.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> CoordinateTransformation::constructor;

void CoordinateTransformation::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(CoordinateTransformation::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("CoordinateTransformation"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "transformPoint", transformPoint);

	target->Set(NanNew("CoordinateTransformation"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

CoordinateTransformation::CoordinateTransformation(OGRCoordinateTransformation *transform)
	: ObjectWrap(),
	  this_(transform)
{
	LOG("Created CoordinateTransformation [%p]", transform);
}

CoordinateTransformation::CoordinateTransformation()
	: ObjectWrap(),
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
	NanScope();
	CoordinateTransformation *f;
	SpatialReference *source, *target;

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f =  static_cast<CoordinateTransformation *>(ptr);
	} else {
		if(args.Length() < 2) {
			NanThrowError("Invalid number of arguments");
			NanReturnUndefined();
		}

		NODE_ARG_WRAPPED(0, "source", SpatialReference, source);

		if(!args[1]->IsObject() || args[1]->IsNull()){
			NanThrowTypeError("target must be a SpatialReference or Dataset object");
			NanReturnUndefined();
		}
		if(NanHasInstance(SpatialReference::constructor, args[1])) {
			// srs -> srs
			NODE_ARG_WRAPPED(1, "target", SpatialReference, target);

			OGRCoordinateTransformation * transform = OGRCreateCoordinateTransformation(source->get(), target->get());
			if (!transform) {
				NODE_THROW_LAST_CPLERR();
				NanReturnUndefined();
			}
			f = new CoordinateTransformation(transform);
		} else if(NanHasInstance(Dataset::constructor, args[1])) {
			// srs -> px/line
			// todo: allow additional options using StringList

			Dataset *ds;
			char** papszTO = NULL;
			char* src_wkt;

			ds = ObjectWrap::Unwrap<Dataset>(args[1].As<Object>());

			if(!ds->getDataset()){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()){
					NanThrowError("Only raster datasets can be used to create geotransform coordinate transformations");
					NanReturnUndefined();
				}
				#endif
				NanThrowError("Dataset already closed");
				NanReturnUndefined();
			}

			OGRErr err = source->get()->exportToWkt(&src_wkt);
			if(err) {
				NODE_THROW_OGRERR(err);
				NanReturnUndefined();
			}

			papszTO = CSLSetNameValue( papszTO, "DST_SRS", src_wkt );
			papszTO = CSLSetNameValue( papszTO, "INSERT_CENTER_LONG", "FALSE" );

			GeoTransformTransformer* transform = new GeoTransformTransformer();
			transform->hSrcImageTransformer = GDALCreateGenImgProjTransformer2( ds->getDataset(), NULL, papszTO );
			if(!transform->hSrcImageTransformer){
				NODE_THROW_LAST_CPLERR();
				NanReturnUndefined();
			}

			f = new CoordinateTransformation(transform);

			CPLFree(src_wkt);
			CSLDestroy(papszTO);
		} else {
			NanThrowTypeError("target must be a SpatialReference or Dataset object");
			NanReturnUndefined();
		}
	}

	f->Wrap(args.This());
	NanReturnValue(args.This());
}

Handle<Value> CoordinateTransformation::New(OGRCoordinateTransformation *transform)
{
	NanEscapableScope();

	if (!transform) {
		return NanEscapeScope(NanNull());
	}

	CoordinateTransformation *wrapped = new CoordinateTransformation(transform);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(CoordinateTransformation::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(CoordinateTransformation::toString)
{
	NanScope();
	NanReturnValue(NanNew("CoordinateTransformation"));
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
	NanScope();

	CoordinateTransformation *transform = ObjectWrap::Unwrap<CoordinateTransformation>(args.This());

	double x, y, z = 0;

	if (args.Length() == 1 && args[0]->IsObject()) {
		Local<Object> obj = args[0].As<Object>();
		Local<Value> arg_x = obj->Get(NanNew("x"));
		Local<Value> arg_y = obj->Get(NanNew("y"));
		Local<Value> arg_z = obj->Get(NanNew("z"));
		if (!arg_x->IsNumber() || !arg_y->IsNumber()) {
			NanThrowError("point must contain numerical properties x and y");
			NanReturnUndefined();
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
		NanThrowError("Error transforming point");
		NanReturnUndefined();
	}

	Local<Object> result = NanNew<Object>();
	result->Set(NanNew("x"), NanNew<Number>(x));
	result->Set(NanNew("y"), NanNew<Number>(y));
	result->Set(NanNew("z"), NanNew<Number>(z));

	NanReturnValue(result);
}

} // namespace node_gdal