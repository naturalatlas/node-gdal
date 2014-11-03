#include "gdal_algorithms.hpp"
#include "gdal_common.hpp"
#include "gdal_layer.hpp"
#include "gdal_dataset.hpp"
#include "gdal_rasterband.hpp"
#include "utils/number_list.hpp"

namespace node_gdal {


void Algorithms::Initialize(Handle<Object> target)
{
	NODE_SET_METHOD(target, "fillNodata", fillNodata);
	NODE_SET_METHOD(target, "contourGenerate", contourGenerate);
	NODE_SET_METHOD(target, "sieveFilter", sieveFilter);
	NODE_SET_METHOD(target, "checksumImage", checksumImage);
	NODE_SET_METHOD(target, "polygonize", polygonize);
}



NAN_METHOD(Algorithms::fillNodata)
{
	NanScope();

	Handle<Object> obj;
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
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(Algorithms::contourGenerate)
{
	NanScope();

	Handle<Object> obj;
	Handle<Value> prop;
	RasterBand* src;
	Layer* dst; 
	double interval = 100, base = 0;
	double *fixed_levels = NULL;
	DoubleList fixed_level_array;
	int n_fixed_levels = 0;
	int use_nodata = 0;
	double nodata = 0;
	int id_field, elev_field;

	NODE_ARG_OBJECT(0, "options", obj);

	NODE_WRAPPED_FROM_OBJ(obj, "src", RasterBand, src);
	NODE_WRAPPED_FROM_OBJ(obj, "dst", Layer, dst);
	NODE_INT_FROM_OBJ(obj, "idField", id_field);
	NODE_INT_FROM_OBJ(obj, "elevField", elev_field);
	NODE_DOUBLE_FROM_OBJ_OPT(obj, "interval", interval);
	NODE_DOUBLE_FROM_OBJ_OPT(obj, "offset", base);
	if(obj->HasOwnProperty(NanNew("fixedLevels"))){
		if(fixed_level_array.parse(obj->Get(NanNew("fixedLevels")))){
			NanReturnUndefined(); //error parsing double list
		} else {
			fixed_levels = fixed_level_array.get();
			n_fixed_levels = fixed_level_array.length();
		}
	}
	if(obj->HasOwnProperty(NanNew("nodata"))){
		prop = obj->Get(NanNew("nodata"));
		if(prop->IsNumber()){
			use_nodata = 1;
			nodata = prop->NumberValue();
		} else if(!prop->IsNull() && !prop->IsUndefined()){
			NanThrowTypeError("nodata property must be a number");
		}
	}

	CPLErr err = GDALContourGenerate(src->get(), interval, base, n_fixed_levels, fixed_levels, use_nodata, nodata, dst->get(), id_field, elev_field, NULL, NULL);
	
	if(err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(Algorithms::sieveFilter)
{
	NanScope();

	Handle<Object> obj;
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
		NanThrowError("connectedness option must be 4 or 8");
		NanReturnUndefined();
	}

	CPLErr err = GDALSieveFilter(src->get(), mask ? mask->get() : NULL, dst->get(), threshold, connectedness, NULL, NULL, NULL);
	
	if(err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(Algorithms::checksumImage)
{
	NanScope();

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
		NanThrowRangeError("offset invalid for given band");
		NanReturnUndefined();
	}
	if(w < 0 || h < 0 || w > bandw || h > bandh){
		NanThrowRangeError("x and y size must be smaller than band dimensions and greater than 0");
		NanReturnUndefined();
	}
	if(x+w-1 >= bandw || y+h-1 >= bandh){
		NanThrowRangeError("given range is outside bounds of given band");
		NanReturnUndefined();
	}

	int checksum = GDALChecksumImage(src->get(), x, y, w, h);
	
	NanReturnValue(NanNew<Integer>(checksum));
}

NAN_METHOD(Algorithms::polygonize)
{
	NanScope();

	Handle<Object> obj;
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
		NanThrowError("connectedness must be 4 or 8");
		NanReturnUndefined();
	}

	CPLErr err;
	if(obj->HasOwnProperty(NanNew("useFloats")) && obj->Get(NanNew("useFloats"))->BooleanValue()){
		err = GDALFPolygonize(src->get(), mask ? mask->get() : NULL, dst->get(), pix_val_field, papszOptions, NULL, NULL);
	} else {
		err = GDALPolygonize(src->get(), mask ? mask->get() : NULL, dst->get(), pix_val_field, papszOptions, NULL, NULL);
	}
	
	if(papszOptions) CSLDestroy(papszOptions);

	if(err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

} //node_gdal namespace