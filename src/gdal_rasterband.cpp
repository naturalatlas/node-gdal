
#include "gdal_common.hpp"

#include "gdal_majorobject.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_dataset.hpp"
#include "collections/rasterband_overviews.hpp"
#include "collections/rasterband_pixels.hpp"

#include <limits>
#include <cpl_port.h>

namespace node_gdal {

Persistent<FunctionTemplate> RasterBand::constructor;
ObjectCache<GDALRasterBand, RasterBand> RasterBand::cache;

void RasterBand::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(RasterBand::New);
	lcons->Inherit(NanNew(MajorObject::constructor));
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("RasterBand"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "flush", flush);
	NODE_SET_PROTOTYPE_METHOD(lcons, "fill", fill);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getStatistics", getStatistics);
	NODE_SET_PROTOTYPE_METHOD(lcons, "setStatistics", setStatistics);
	NODE_SET_PROTOTYPE_METHOD(lcons, "computeStatistics", computeStatistics);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getMaskBand", getMaskBand);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getMaskFlags", getMaskFlags);
	NODE_SET_PROTOTYPE_METHOD(lcons, "createMaskBand", createMaskBand);

	// unimplemented methods
	//NODE_SET_PROTOTYPE_METHOD(lcons, "buildOverviews", buildOverviews);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "rasterIO", rasterIO);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "getColorTable", getColorTable);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "setColorTable", setColorTable);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "getHistogram", getHistogram);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "getDefaultHistogram", getDefaultHistogram);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "setDefaultHistogram", setDefaultHistogram);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "id", idGetter, READ_ONLY_SETTER);
	ATTR(lcons, "size", sizeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "overviews", overviewsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "pixels", pixelsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "blockSize", blockSizeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "minimum", minimumGetter, READ_ONLY_SETTER);
	ATTR(lcons, "maximum", maximumGetter, READ_ONLY_SETTER);
	ATTR(lcons, "readOnly", readOnlyGetter, READ_ONLY_SETTER);
	ATTR(lcons, "dataType", dataTypeGetter, READ_ONLY_SETTER);
	ATTR(lcons, "hasArbitraryOverviews", hasArbitraryOverviewsGetter, READ_ONLY_SETTER);
	ATTR(lcons, "unitType", unitTypeGetter, unitTypeSetter);
	ATTR(lcons, "scale", scaleGetter, scaleSetter);
	ATTR(lcons, "offset", offsetGetter, offsetSetter);
	ATTR(lcons, "noDataValue", noDataValueGetter, noDataValueSetter);
	ATTR(lcons, "categoryNames", categoryNamesGetter, categoryNamesSetter);
	ATTR(lcons, "colorInterpretation", colorInterpretationGetter, colorInterpretationSetter);

	target->Set(NanNew("RasterBand"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

RasterBand::RasterBand(GDALRasterBand *band)
	: ObjectWrap(), this_(band), parent_ds(0)
{
	LOG("Created band [%p] (dataset = %p)", band, band->GetDataset());
}

RasterBand::RasterBand()
	: ObjectWrap(), this_(0), parent_ds(0)
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
		LOG("Disposing band [%p]", this_);

		cache.erase(this_);

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

		LOG("Disposed band [%p]", this_);

		this_ = NULL;
	}
}

/**
 * A single raster band (or channel).
 *
 * @class gdal.RasterBand
 */
NAN_METHOD(RasterBand::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		RasterBand *f = static_cast<RasterBand *>(ptr);
		f->Wrap(args.This());

		Handle<Value> overviews = RasterBandOverviews::New(args.This());
		args.This()->SetHiddenValue(NanNew("overviews_"), overviews);
		Handle<Value> pixels = RasterBandPixels::New(args.This());
		args.This()->SetHiddenValue(NanNew("pixels_"), pixels);

		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create band directly create with dataset instead");
		NanReturnUndefined();
	}
}
Handle<Value> RasterBand::New(GDALRasterBand *raw, GDALDataset *raw_parent)
{
	NanEscapableScope();

	if (!raw) {
		return NanEscapeScope(NanNull());
	}
	if (cache.has(raw)) {
		return NanEscapeScope(NanNew(cache.get(raw)));
	}

	RasterBand *wrapped = new RasterBand(raw);

	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(RasterBand::constructor)->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	//add reference to dataset so dataset doesnt get GC'ed while band is alive
	if (raw_parent) {
		//DONT USE GDALRasterBand.GetDataset() ... it will return a "fake" dataset for overview bands
		//https://github.com/naturalatlas/node-gdal/blob/master/deps/libgdal/gdal/frmts/gtiff/geotiff.cpp#L84

		Handle<Value> ds;
		if (Dataset::dataset_cache.has(raw_parent)) {
			ds = NanNew(Dataset::dataset_cache.get(raw_parent));
		} else {
			LOG("Band's parent dataset disappeared from cache (band = %p, dataset = %p)", raw, raw_parent);
			NanThrowError("Band's parent dataset disappeared from cache");
			return NanEscapeScope(NanUndefined());
			//ds = Dataset::New(raw_parent); //this should never happen
		}

		wrapped->parent_ds = raw_parent;
		obj->SetHiddenValue(NanNew("ds_"), ds);
	}

	return NanEscapeScope(obj);
}

NAN_METHOD(RasterBand::toString)
{
	NanScope();
	NanReturnValue(NanNew("RasterBand"));
}

/**
 * Saves changes to disk.
 *
 * @method flush
 */
NODE_WRAPPED_METHOD(RasterBand, flush, FlushCache);

/**
 * Return the status flags of the mask band associated with the band.
 *
 * The result will be a bitwise OR-ed set of status flags with the following
 * available definitions that may be extended in the future:
 *
 *   - `GMF_ALL_VALID` (`0x01`): There are no invalid pixels, all mask values will be 255. When used this will normally be the only flag set.
 *   - `GMF_PER_DATASET` (`0x02`): The mask band is shared between all bands on the dataset.
 *   - `GMF_ALPHA` (`0x04`): The mask band is actually an alpha band and may have values other than 0 and 255.
 *   - `GMF_NODATA` (`0x08`): Indicates the mask is actually being generated from nodata values. (mutually exclusive of `GMF_ALPHA`)
 *
 * @method getMaskFlags
 * @return {Integer} Mask flags
 */
NODE_WRAPPED_METHOD_WITH_RESULT(RasterBand, getMaskFlags, Integer, GetMaskFlags);
// TODO: expose GMF constants in API
// ({{#crossLink "Constants (GMF)"}}see flags{{/crossLink}})

/**
 * Adds a mask band to the current band.
 *
 * @throws Error
 * @method createMaskBand
 * @param {Integer} flags Mask flags
 */
NODE_WRAPPED_METHOD_WITH_CPLERR_RESULT_1_INTEGER_PARAM(RasterBand, createMaskBand, CreateMaskBand, "mask flags");
// TODO: expose GMF constants in API
// ({{#crossLink "Constants (GMF)"}}see flags{{/crossLink}})

/**
 * Return the mask band associated with the band.
 *
 * @method getMaskBand
 * @return {gdal.RasterBand}
 */
NAN_METHOD(RasterBand::getMaskBand)
{
	NanScope();

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	GDALRasterBand *mask_band = band->this_->GetMaskBand();

	if(!mask_band) NanReturnNull();

	NanReturnValue(RasterBand::New(mask_band, band->getParent()));
}

/**
 * Fill this band with a constant value.
 *
 * @throws Error
 * @method fill
 * @param {Number} real_value
 * @param {Number} [imaginary_value]
 */
NAN_METHOD(RasterBand::fill)
{
	NanScope();
	double real, imaginary = 0;
	NODE_ARG_DOUBLE(0, "real value", real);
	NODE_ARG_DOUBLE_OPT(1, "imaginary value", real);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int err = band->this_->Fill(real, imaginary);

	if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}

// --- Custom error handling to handle VRT errors ---
// see: https://github.com/mapbox/mapnik-omnivore/issues/10

std::string stats_file_err = "";
CPLErrorHandler last_err_handler;
void CPL_STDCALL statisticsErrorHandler(CPLErr eErrClass, int err_no, const char *msg) {
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

/**
 * Fetch image statistics.
 *
 * Returns the minimum, maximum, mean and standard deviation of all pixel values
 * in this band. If approximate statistics are sufficient, the `allow_approximation`
 * argument can be set to `true` in which case overviews, or a subset of image tiles
 * may be used in computing the statistics.
 *
 * @throws Error
 * @method getStatistics
 * @param {Boolean} allow_approximation If `true` statistics may be computed based on overviews or a subset of all tiles.
 * @param {Boolean} force If `false` statistics will only be returned if it can be done without rescanning the image.
 * @return {Object} Statistics containing `"min"`, `"max"`, `"mean"`, `"std_dev"` properties.
 */
NAN_METHOD(RasterBand::getStatistics)
{
	NanScope();
	double min, max, mean, std_dev;
	int approx, force;
	NODE_ARG_BOOL(0, "allow approximation", approx);
	NODE_ARG_BOOL(1, "force", force);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	pushStatsErrorHandler();
	CPLErr err = band->this_->GetStatistics(approx, force, &min, &max, &mean, &std_dev);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		NanThrowError(stats_file_err.c_str());
	} else if (err) {
		if (!force && err == CE_Warning) {
			NanThrowError("Statistics cannot be efficiently computed without scanning raster");
			NanReturnUndefined();
		}
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	Local<Object> result = NanNew<Object>();
	result->Set(NanNew("min"), NanNew<Number>(min));
	result->Set(NanNew("max"), NanNew<Number>(max));
	result->Set(NanNew("mean"), NanNew<Number>(mean));
	result->Set(NanNew("std_dev"), NanNew<Number>(std_dev));

	NanReturnValue(result);
}

/**
 * Computes image statistics.
 *
 * Returns the minimum, maximum, mean and standard deviation of all pixel values
 * in this band. If approximate statistics are sufficient, the `allow_approximation`
 * argument can be set to `true` in which case overviews, or a subset of image tiles
 * may be used in computing the statistics.
 *
 * @throws Error
 * @method getStatistics
 * @param {Boolean} allow_approximation If `true` statistics may be computed based on overviews or a subset of all tiles.
 * @return {Object} Statistics containing `"min"`, `"max"`, `"mean"`, `"std_dev"` properties.
 */
NAN_METHOD(RasterBand::computeStatistics)
{
	NanScope();
	double min, max, mean, std_dev;
	int approx;
	NODE_ARG_BOOL(0, "allow approximation", approx);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	pushStatsErrorHandler();
	CPLErr err = band->this_->ComputeStatistics(approx, &min, &max, &mean, &std_dev, NULL, NULL);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		NanThrowError(stats_file_err.c_str());
	} else if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	Local<Object> result = NanNew<Object>();
	result->Set(NanNew("min"), NanNew<Number>(min));
	result->Set(NanNew("max"), NanNew<Number>(max));
	result->Set(NanNew("mean"), NanNew<Number>(mean));
	result->Set(NanNew("std_dev"), NanNew<Number>(std_dev));

	NanReturnValue(result);
}

/**
 * Set statistics on the band. This method can be used to store
 * min/max/mean/standard deviation statistics.
 *
 * @throws Error
 * @method setStatistics
 * @param {Number} min
 * @param {Number} max
 * @param {Number} mean
 * @param {Number} std_dev
 */
NAN_METHOD(RasterBand::setStatistics)
{
	NanScope();
	double min, max, mean, std_dev;

	NODE_ARG_DOUBLE(0, "min", min);
	NODE_ARG_DOUBLE(1, "max", max);
	NODE_ARG_DOUBLE(2, "mean", mean);
	NODE_ARG_DOUBLE(3, "standard deviation", std_dev);

	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	CPLErr err = band->this_->SetStatistics(min, max, mean, std_dev);

	if (err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}

/**
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(RasterBand::dsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("ds_")));
}

/**
 * @readOnly
 * @attribute overviews
 * @type {gdal.RasterBandOverviews}
 */
NAN_GETTER(RasterBand::overviewsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("overviews_")));
}

/**
 * @readOnly
 * @attribute pixels
 * @type {gdal.RasterBandPixels}
 */
NAN_GETTER(RasterBand::pixelsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("pixels_")));
}

/**
 * @readOnly
 * @attribute id
 * @type {Integer|null}
 */
NAN_GETTER(RasterBand::idGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int id = band->this_->GetBand();

	if(id == 0) {
		NanReturnNull();
	} else {
		NanReturnValue(NanNew<Integer>(id));
	}
}

/**
 * Size object containing `"x"` and `"y"` properties.
 *
 * @readOnly
 * @attribute size
 * @type {Object}
 */
NAN_GETTER(RasterBand::sizeGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	Local<Object> result = NanNew<Object>();
	result->Set(NanNew("x"), NanNew<Integer>(band->this_->GetXSize()));
	result->Set(NanNew("y"), NanNew<Integer>(band->this_->GetYSize()));
	NanReturnValue(result);
}

/**
 * Size object containing `"x"` and `"y"` properties.
 *
 * @readOnly
 * @attribute blockSize
 * @type {Object}
 */
NAN_GETTER(RasterBand::blockSizeGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int x, y;
	band->this_->GetBlockSize(&x, &y);

	Local<Object> result = NanNew<Object>();
	result->Set(NanNew("x"), NanNew<Integer>(x));
	result->Set(NanNew("y"), NanNew<Integer>(y));
	NanReturnValue(result);
}

/**
 * Minimum value for this band.
 *
 * @readOnly
 * @attribute minimum
 * @type {Number}
 */
NAN_GETTER(RasterBand::minimumGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int success = 0;
	double result = band->this_->GetMinimum(&success);
	NanReturnValue(NanNew<Number>(result));
}

/**
 * Maximum value for this band.
 *
 * @readOnly
 * @attribute maximum
 * @type {Number}
 */
NAN_GETTER(RasterBand::maximumGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int success = 0;
	double result = band->this_->GetMaximum(&success);
	NanReturnValue(NanNew<Number>(result));
}

/**
 * Raster value offset.
 *
 * @attribute offset
 * @type {Number}
 */
NAN_GETTER(RasterBand::offsetGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int success = 0;
	double result = band->this_->GetOffset(&success);
	NanReturnValue(NanNew<Number>(result));
}

/**
 * Raster value scale.
 *
 * @attribute scale
 * @type {Number}
 */
NAN_GETTER(RasterBand::scaleGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int success = 0;
	double result = band->this_->GetScale(&success);
	NanReturnValue(NanNew<Number>(result));
}

/**
 * No data value for this band.
 *
 * @attribute noDataValue
 * @type {Number|null}
 */
NAN_GETTER(RasterBand::noDataValueGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	int success = 0;
	double result = band->this_->GetNoDataValue(&success);

	if(success && !CPLIsNan(result)) {
		NanReturnValue(NanNew<Number>(result));
	} else {
		NanReturnNull();
	}
}

/**
 * Raster unit type (name for the units of this raster's values).
 * For instance, it might be `"m"` for an elevation model in meters,
 * or `"ft"` for feet. If no units are available, a value of `""`
 * will be returned.
 *
 * @attribute unitType
 * @type {String}
 */
NAN_GETTER(RasterBand::unitTypeGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	const char *result = band->this_->GetUnitType();
	NanReturnValue(SafeString::New(result));
}

/**
 * Pixel data type ({{#crossLink "Constants (GDT)"}}see GDT constants{{/crossLink}}) used for this band.
 *
 * @readOnly
 * @attribute dataType
 * @type {String|Undefined}
 */
NAN_GETTER(RasterBand::dataTypeGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	GDALDataType type = band->this_->GetRasterDataType();

	if(type == GDT_Unknown) NanReturnUndefined();
	NanReturnValue(SafeString::New(GDALGetDataTypeName(type)));
}

/**
 * Indicates if the band is read-only.
 *
 * @readOnly
 * @attribute readOnly
 * @type {Boolean}
 */
NAN_GETTER(RasterBand::readOnlyGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	GDALAccess result = band->this_->GetAccess();
	NanReturnValue(result == GA_Update ? NanFalse() : NanTrue());
}

/**
 * An indicator if the underlying datastore can compute arbitrary overviews
 * efficiently, such as is the case with OGDI over a network. Datastores with
 * arbitrary overviews don't generally have any fixed overviews, but GDAL's
 * `RasterIO()` method can be used in downsampling mode to get overview
 * data efficiently.
 *
 * @readOnly
 * @attribute hasArbitraryOverviews
 * @type {Boolean}
 */
NAN_GETTER(RasterBand::hasArbitraryOverviewsGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	bool result = band->this_->HasArbitraryOverviews();
	NanReturnValue(NanNew<Boolean>(result));
}

/**
 * List of list of category names for this raster.
 *
 * @attribute categoryNames
 * @type {Array}
 */
NAN_GETTER(RasterBand::categoryNamesGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}

	char ** names = band->this_->GetCategoryNames();

	Handle<Array> results = NanNew<Array>();

	if (names) {
		int i = 0;
		while (names[i]) {
			results->Set(i, NanNew(names[i]));
			i++;
		}
	}

	NanReturnValue(results);
}

/**
 * Color interpretation mode ({{#crossLink "Constants (GCI)"}}see GCI constants{{/crossLink}}).
 *
 * @attribute colorInterpretation
 * @type {string}
 */
NAN_GETTER(RasterBand::colorInterpretationGetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		NanReturnUndefined();
	}
	GDALColorInterp interp = band->this_->GetColorInterpretation();
	if(interp == GCI_Undefined) NanReturnUndefined();
	else NanReturnValue(SafeString::New(GDALGetColorInterpretationName(interp)));
}

NAN_SETTER(RasterBand::unitTypeSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsString()) {
		NanThrowError("Unit type must be a string");
		return;
	}
	std::string input = *NanUtf8String(value);
	CPLErr err = band->this_->SetUnitType(input.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::noDataValueSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	double input;

	if (value->IsNull() || value -> IsUndefined()){
		input = std::numeric_limits<double>::quiet_NaN();
	} else if (value->IsNumber()) {
		input = value->NumberValue();
	} else {
		NanThrowError("No data value must be a number");
		return;
	}

	CPLErr err = band->this_->SetNoDataValue(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::scaleSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsNumber()) {
		NanThrowError("Scale must be a number");
		return;
	}
	double input = value->NumberValue();
	CPLErr err = band->this_->SetScale(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::offsetSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsNumber()) {
		NanThrowError("Offset must be a number");
		return;
	}
	double input = value->NumberValue();
	CPLErr err = band->this_->SetOffset(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::categoryNamesSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	if(!value->IsArray()){
		NanThrowError("Category names must be an array");
		return;
	}
	Handle<Array> names = value.As<Array>();

	char **list = NULL;
	std::string *strlist = NULL;

	if (names->Length() > 0) {
		list = new char* [names->Length() + 1];
		strlist = new std::string [names->Length()];
		unsigned int i;
		for (i = 0; i < names->Length(); i++) {
			strlist[i] = *NanUtf8String(names->Get(i));
			list[i] = (char*) strlist[i].c_str();
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

NAN_SETTER(RasterBand::colorInterpretationSetter)
{
	NanScope();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(args.This());
	if (!band->this_) {
		NanThrowError("RasterBand object has already been destroyed");
		return;
	}

	GDALColorInterp ci = GCI_Undefined;

	if (value->IsString()) {
		std::string name = *NanUtf8String(value);
		ci = GDALGetColorInterpretationByName(name.c_str());
	} else if(!value->IsNull() && !value->IsUndefined()) {
		NanThrowError("color interpretation must be a string or undefined");
		return;
	}

	CPLErr err = band->this_->SetColorInterpretation(ci);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

} // namespace node_gdal