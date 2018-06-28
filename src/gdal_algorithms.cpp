#include "gdal_algorithms.hpp"
#include "gdal_common.hpp"
#include "gdal_layer.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"
#include "utils/number_list.hpp"

namespace node_gdal {

void Algorithms::Initialize(Local<Object> target)
{
	Nan::SetMethod(target, "fillNodata", fillNodata);
	Nan::SetMethod(target, "contourGenerate", contourGenerate);
	Nan::SetMethod(target, "sieveFilter", sieveFilter);
	Nan::SetMethod(target, "checksumImage", checksumImage);
	Nan::SetMethod(target, "polygonize", polygonize);
}

/**
 * Fill raster regions by interpolation from edges.
 *
 * @throws Error
 * @method fillNodata
 * @static
 * @for gdal
 * @param {Object} options
 * @param {gdal.RasterBand} options.src This band to be updated in-place.
 * @param {gdal.RasterBand} [options.mask] Mask band
 * @param {Number} options.searchDist The maximum distance (in pixels) that the algorithm will search out for values to interpolate.
 * @param {integer} [options.smoothingIterations=0] The number of 3x3 average filter smoothing iterations to run after the interpolation to dampen artifacts.
 */
NAN_METHOD(Algorithms::fillNodata)
{
	Nan::HandleScope scope;

	Local<Object> obj;
	RasterBand* src;
	RasterBand* mask = NULL;
	double search_dist;
	int smooth_iterations = 0;

	NODE_ARG_OBJECT(0, "options", obj);

	NODE_WRAPPED_FROM_OBJ(obj, "src", RasterBand, src);
	NODE_WRAPPED_FROM_OBJ_OPT(obj, "mask", RasterBand, mask);
	NODE_DOUBLE_FROM_OBJ(obj, "searchDist", search_dist);
	NODE_INT_FROM_OBJ_OPT(obj, "smoothIterations", smooth_iterations)

	CPLErr err = GDALFillNodata(src->get(), mask ? mask->get() : NULL, search_dist, 0, smooth_iterations, NULL, NULL, NULL);

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Create vector contours from raster DEM.
 *
 * This algorithm will generate contour vectors for the input raster band on the
 * requested set of contour levels. The vector contours are written to the passed
 * in vector layer. Also, a NODATA value may be specified to identify pixels
 * that should not be considered in contour line generation.
 *
 * @throws Error
 * @method contourGenerate
 * @static
 * @for gdal
 * @param {Object} options
 * @param {gdal.RasterBand} options.src
 * @param {gdal.Layer} options.dst
 * @param {Number} [options.offset=0] The "offset" relative to which contour intervals are applied. This is normally zero, but could be different. To generate 10m contours at 5, 15, 25, ... the offset would be 5.
 * @param {Number} [options.interval=100] The elevation interval between contours generated.
 * @param {Number[]} [options.fixedLevels] A list of fixed contour levels at which contours should be generated. Overrides interval/base options if set.
 * @param {Number} [options.nodata] The value to use as a "nodata" value. That is, a pixel value which should be ignored in generating contours as if the value of the pixel were not known.
 * @param {integer} [options.idField] A field index to indicate where a unique id should be written for each feature (contour) written.
 * @param {integer} [options.elevField] A field index to indicate where the elevation value of the contour should be written.
 */
NAN_METHOD(Algorithms::contourGenerate)
{
	Nan::HandleScope scope;

	Local<Object> obj;
	Local<Value> prop;
	RasterBand* src;
	Layer* dst;
	double interval = 100, base = 0;
	double *fixed_levels = NULL;
	DoubleList fixed_level_array;
	int n_fixed_levels = 0;
	int use_nodata = 0;
	double nodata = 0;
	int id_field = -1, elev_field = -1;

	NODE_ARG_OBJECT(0, "options", obj);

	NODE_WRAPPED_FROM_OBJ(obj, "src", RasterBand, src);
	NODE_WRAPPED_FROM_OBJ(obj, "dst", Layer, dst);
	NODE_INT_FROM_OBJ_OPT(obj, "idField", id_field);
	NODE_INT_FROM_OBJ_OPT(obj, "elevField", elev_field);
	NODE_DOUBLE_FROM_OBJ_OPT(obj, "interval", interval);
	NODE_DOUBLE_FROM_OBJ_OPT(obj, "offset", base);
	if(Nan::HasOwnProperty(obj, Nan::New("fixedLevels").ToLocalChecked()).FromMaybe(false)){
		if(fixed_level_array.parse(obj->Get(Nan::New("fixedLevels").ToLocalChecked()))){
			return; //error parsing double list
		} else {
			fixed_levels = fixed_level_array.get();
			n_fixed_levels = fixed_level_array.length();
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("nodata").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("nodata").ToLocalChecked());
		if(prop->IsNumber()){
			use_nodata = 1;
			nodata = prop->NumberValue();
		} else if(!prop->IsNull() && !prop->IsUndefined()){
			Nan::ThrowTypeError("nodata property must be a number");
		}
	}

	CPLErr err = GDALContourGenerate(src->get(), interval, base, n_fixed_levels, fixed_levels, use_nodata, nodata, dst->get(), id_field, elev_field, NULL, NULL);

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Removes small raster polygons.
 *
 * @throws Error
 * @method sieveFilter
 * @static
 * @for gdal
 * @param {Object} options
 * @param {gdal.RasterBand} options.src
 * @param {gdal.RasterBand} options.dst Output raster band. It may be the same as src band to update the source in place.
 * @param {gdal.RasterBand} [options.mask] All pixels in the mask band with a value other than zero will be considered suitable for inclusion in polygons.
 * @param {Number} options.threshold Raster polygons with sizes smaller than this will be merged into their largest neighbour.
 * @param {integer} [options.connectedness=4] Either 4 indicating that diagonal pixels are not considered directly adjacent for polygon membership purposes or 8 indicating they are.
 */
NAN_METHOD(Algorithms::sieveFilter)
{
	Nan::HandleScope scope;

	Local<Object> obj;
	RasterBand* src;
	RasterBand* dst;
	RasterBand* mask = NULL;
	int threshold;
	int connectedness = 4;

	NODE_ARG_OBJECT(0, "options", obj);

	NODE_WRAPPED_FROM_OBJ(obj, "src", RasterBand, src);
	NODE_WRAPPED_FROM_OBJ(obj, "dst", RasterBand, dst);
	NODE_WRAPPED_FROM_OBJ_OPT(obj, "mask", RasterBand, mask);
	NODE_INT_FROM_OBJ(obj, "threshold", threshold);
	NODE_INT_FROM_OBJ_OPT(obj, "connectedness", connectedness);

	if(connectedness != 4 && connectedness != 8){
		Nan::ThrowError("connectedness option must be 4 or 8");
		return;
	}

	CPLErr err = GDALSieveFilter(src->get(), mask ? mask->get() : NULL, dst->get(), threshold, connectedness, NULL, NULL, NULL);

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Compute checksum for image region.
 *
 * @throws Error
 * @method checksumImage
 * @static
 * @for gdal
 * @param {gdal.RasterBand} src
 * @param {integer} [x=0]
 * @param {integer} [y=0]
 * @param {integer} [w=src.width]
 * @param {integer} [h=src.height]
 * @return integer
 */
NAN_METHOD(Algorithms::checksumImage)
{
	Nan::HandleScope scope;

	RasterBand* src;
	int x = 0, y = 0, w, h, bandw, bandh;

	NODE_ARG_WRAPPED(0, "src", RasterBand, src);

	w = bandw = src->get()->GetXSize();
	h = bandh = src->get()->GetYSize();

	NODE_ARG_INT_OPT(1, "x", x);
	NODE_ARG_INT_OPT(2, "y", y);
	NODE_ARG_INT_OPT(3, "xSize", w);
	NODE_ARG_INT_OPT(4, "ySize", h);

	if(x < 0 || y < 0 || x >= bandw || y >= bandh){
		Nan::ThrowRangeError("offset invalid for given band");
		return;
	}
	if(w < 0 || h < 0 || w > bandw || h > bandh){
		Nan::ThrowRangeError("x and y size must be smaller than band dimensions and greater than 0");
		return;
	}
	if(x+w-1 >= bandw || y+h-1 >= bandh){
		Nan::ThrowRangeError("given range is outside bounds of given band");
		return;
	}

	int checksum = GDALChecksumImage(src->get(), x, y, w, h);

	info.GetReturnValue().Set(Nan::New<Integer>(checksum));
}

/**
 * Creates vector polygons for all connected regions of pixels in the raster
 * sharing a common pixel value. Each polygon is created with an attribute
 * indicating the pixel value of that polygon. A raster mask may also be
 * provided to determine which pixels are eligible for processing.
 *
 * @throws Error
 * @method polygonize
 * @static
 * @for gdal
 * @param {Object} options
 * @param {gdal.RasterBand} options.src
 * @param {gdal.Layer} options.dst
 * @param {gdal.RasterBand} [options.mask]
 * @param {integer} options.pixValField The attribute field index indicating the feature attribute into which the pixel value of the polygon should be written.
 * @param {integer} [options.connectedness=4] Either 4 indicating that diagonal pixels are not considered directly adjacent for polygon membership purposes or 8 indicating they are.
 * @param {Boolean} [options.useFloats=false] Use floating point buffers instead of int buffers.
 */
NAN_METHOD(Algorithms::polygonize)
{
	Nan::HandleScope scope;

	Local<Object> obj;
	RasterBand* src;
	RasterBand* mask = NULL;
	Layer* dst;
	int connectedness = 4;
	int pix_val_field = 0;
	char** papszOptions = NULL;

	NODE_ARG_OBJECT(0, "options", obj);

	NODE_WRAPPED_FROM_OBJ(obj, "src", RasterBand, src);
	NODE_WRAPPED_FROM_OBJ(obj, "dst", Layer, dst);
	NODE_WRAPPED_FROM_OBJ_OPT(obj, "mask", RasterBand, mask);
	NODE_INT_FROM_OBJ_OPT(obj, "connectedness", connectedness)
	NODE_INT_FROM_OBJ(obj, "pixValField", pix_val_field);

	if(connectedness == 8) {
		papszOptions = CSLSetNameValue(papszOptions, "8CONNECTED", "8");
	} else if (connectedness != 4) {
		Nan::ThrowError("connectedness must be 4 or 8");
		return;
	}

	CPLErr err;
	if(Nan::HasOwnProperty(obj, Nan::New("useFloats").ToLocalChecked()).FromMaybe(false) && obj->Get(Nan::New("useFloats").ToLocalChecked())->BooleanValue()){
		err = GDALFPolygonize(src->get(), mask ? mask->get() : NULL, reinterpret_cast<OGRLayerH>(dst->get()), pix_val_field, papszOptions, NULL, NULL);
	} else {
		err = GDALPolygonize(src->get(), mask ? mask->get() : NULL, reinterpret_cast<OGRLayerH>(dst->get()), pix_val_field, papszOptions, NULL, NULL);
	}

	if(papszOptions) CSLDestroy(papszOptions);

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

} //node_gdal namespace