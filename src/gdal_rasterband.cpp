
#include "gdal_common.hpp"

#include "gdal_majorobject.hpp"
#include "gdal_rasterband.hpp"
#include "gdal_dataset.hpp"
#include "collections/rasterband_overviews.hpp"
#include "collections/rasterband_pixels.hpp"

#include <limits>
#include <cpl_port.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> RasterBand::constructor;
ObjectCache<GDALRasterBand, RasterBand> RasterBand::cache;

void RasterBand::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(RasterBand::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("RasterBand").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "flush", flush);
	Nan::SetPrototypeMethod(lcons, "fill", fill);
	Nan::SetPrototypeMethod(lcons, "getStatistics", getStatistics);
	Nan::SetPrototypeMethod(lcons, "setStatistics", setStatistics);
	Nan::SetPrototypeMethod(lcons, "computeStatistics", computeStatistics);
	Nan::SetPrototypeMethod(lcons, "getMaskBand", getMaskBand);
	Nan::SetPrototypeMethod(lcons, "getMaskFlags", getMaskFlags);
	Nan::SetPrototypeMethod(lcons, "createMaskBand", createMaskBand);
	Nan::SetPrototypeMethod(lcons, "getMetadata", getMetadata);

	// unimplemented methods
	//Nan::SetPrototypeMethod(lcons, "buildOverviews", buildOverviews);
	//Nan::SetPrototypeMethod(lcons, "rasterIO", rasterIO);
	//Nan::SetPrototypeMethod(lcons, "getColorTable", getColorTable);
	//Nan::SetPrototypeMethod(lcons, "setColorTable", setColorTable);
	//Nan::SetPrototypeMethod(lcons, "getHistogram", getHistogram);
	//Nan::SetPrototypeMethod(lcons, "getDefaultHistogram", getDefaultHistogram);
	//Nan::SetPrototypeMethod(lcons, "setDefaultHistogram", setDefaultHistogram);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR_DONT_ENUM(lcons, "_uid", uidGetter, READ_ONLY_SETTER);
	ATTR(lcons, "id", idGetter, READ_ONLY_SETTER);
	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);
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

	target->Set(Nan::New("RasterBand").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

RasterBand::RasterBand(GDALRasterBand *band)
	: Nan::ObjectWrap(), uid(0), this_(band), parent_ds(0)
{
	LOG("Created band [%p] (dataset = %p)", band, band->GetDataset());
}

RasterBand::RasterBand()
	: Nan::ObjectWrap(), uid(0), this_(0), parent_ds(0)
{
}

RasterBand::~RasterBand()
{
	dispose();
}

void RasterBand::dispose()
{
	if (this_) {
		LOG("Disposing band [%p]", this_);

		ptr_manager.dispose(uid);

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
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		RasterBand *f = static_cast<RasterBand *>(ptr);
		f->Wrap(info.This());

		Local<Value> overviews = RasterBandOverviews::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("overviews_").ToLocalChecked(), overviews);
		Local<Value> pixels = RasterBandPixels::New(info.This());
		Nan::SetPrivate(info.This(), Nan::New("pixels_").ToLocalChecked(), pixels);

		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create band directly create with dataset instead");
		return;
	}
}
Local<Value> RasterBand::New(GDALRasterBand *raw, GDALDataset *raw_parent)
{
	Nan::EscapableHandleScope scope;

	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	if (cache.has(raw)) {
		return scope.Escape(cache.get(raw));
	}

	RasterBand *wrapped = new RasterBand(raw);

	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(RasterBand::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	LOG("Adding band to cache[%p] (parent=%p)", raw, raw_parent);
	cache.add(raw, obj);

	//add reference to dataset so dataset doesnt get GC'ed while band is alive
		//DONT USE GDALRasterBand.GetDataset() ... it will return a "fake" dataset for overview bands
		//https://github.com/naturalatlas/node-gdal/blob/master/deps/libgdal/gdal/frmts/gtiff/geotiff.cpp#L84

	Local<Object> ds;
	if (Dataset::dataset_cache.has(raw_parent)) {
		ds = Dataset::dataset_cache.get(raw_parent);
	} else {
		LOG("Band's parent dataset disappeared from cache (band = %p, dataset = %p)", raw, raw_parent);
		Nan::ThrowError("Band's parent dataset disappeared from cache");
		return scope.Escape(Nan::Undefined());
		//ds = Dataset::New(raw_parent); //this should never happen
	}

	long parent_uid = Nan::ObjectWrap::Unwrap<Dataset>(ds)->uid;
	wrapped->uid = ptr_manager.add(raw, parent_uid);
	wrapped->parent_ds = raw_parent;
	Nan::SetPrivate(obj, Nan::New("ds_").ToLocalChecked(), ds);

	return scope.Escape(obj);
}

NAN_METHOD(RasterBand::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("RasterBand").ToLocalChecked());
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
	Nan::HandleScope scope;

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	GDALRasterBand *mask_band = band->this_->GetMaskBand();

	if(!mask_band) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	info.GetReturnValue().Set(RasterBand::New(mask_band, band->getParent()));
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
	Nan::HandleScope scope;
	double real, imaginary = 0;
	NODE_ARG_DOUBLE(0, "real value", real);
	NODE_ARG_DOUBLE_OPT(1, "imaginary value", real);

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int err = band->this_->Fill(real, imaginary);

	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}
	return;
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
	Nan::HandleScope scope;
	double min, max, mean, std_dev;
	int approx, force;
	NODE_ARG_BOOL(0, "allow approximation", approx);
	NODE_ARG_BOOL(1, "force", force);

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	pushStatsErrorHandler();
	CPLErr err = band->this_->GetStatistics(approx, force, &min, &max, &mean, &std_dev);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		Nan::ThrowError(stats_file_err.c_str());
	} else if (err) {
		if (!force && err == CE_Warning) {
			Nan::ThrowError("Statistics cannot be efficiently computed without scanning raster");
			return;
		}
		NODE_THROW_CPLERR(err);
		return;
	}

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("min").ToLocalChecked(), Nan::New<Number>(min));
	result->Set(Nan::New("max").ToLocalChecked(), Nan::New<Number>(max));
	result->Set(Nan::New("mean").ToLocalChecked(), Nan::New<Number>(mean));
	result->Set(Nan::New("std_dev").ToLocalChecked(), Nan::New<Number>(std_dev));

	info.GetReturnValue().Set(result);
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
 * @method computeStatistics
 * @param {Boolean} allow_approximation If `true` statistics may be computed based on overviews or a subset of all tiles.
 * @return {Object} Statistics containing `"min"`, `"max"`, `"mean"`, `"std_dev"` properties.
 */
NAN_METHOD(RasterBand::computeStatistics)
{
	Nan::HandleScope scope;
	double min, max, mean, std_dev;
	int approx;
	NODE_ARG_BOOL(0, "allow approximation", approx);

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	pushStatsErrorHandler();
	CPLErr err = band->this_->ComputeStatistics(approx, &min, &max, &mean, &std_dev, NULL, NULL);
	popStatsErrorHandler();
	if (!stats_file_err.empty()){
		Nan::ThrowError(stats_file_err.c_str());
	} else if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("min").ToLocalChecked(), Nan::New<Number>(min));
	result->Set(Nan::New("max").ToLocalChecked(), Nan::New<Number>(max));
	result->Set(Nan::New("mean").ToLocalChecked(), Nan::New<Number>(mean));
	result->Set(Nan::New("std_dev").ToLocalChecked(), Nan::New<Number>(std_dev));

	info.GetReturnValue().Set(result);
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
	Nan::HandleScope scope;
	double min, max, mean, std_dev;

	NODE_ARG_DOUBLE(0, "min", min);
	NODE_ARG_DOUBLE(1, "max", max);
	NODE_ARG_DOUBLE(2, "mean", mean);
	NODE_ARG_DOUBLE(3, "standard deviation", std_dev);

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	CPLErr err = band->this_->SetStatistics(min, max, mean, std_dev);

	if (err) {
		NODE_THROW_CPLERR(err);
		return;
	}
	return;
}

/**
 * Returns band metadata
 *
 * @method getMetadata
 * @param {string} [domain]
 */
NAN_METHOD(RasterBand::getMetadata)
{
	Nan::HandleScope scope;

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(MajorObject::getMetadata(band->this_, domain.empty() ? NULL : domain.c_str()));
}

/**
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(RasterBand::dsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("ds_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute overviews
 * @type {gdal.RasterBandOverviews}
 */
NAN_GETTER(RasterBand::overviewsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("overviews_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute pixels
 * @type {gdal.RasterBandPixels}
 */
NAN_GETTER(RasterBand::pixelsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("pixels_").ToLocalChecked()).ToLocalChecked());
}

/**
 * @readOnly
 * @attribute id
 * @type {Integer|null}
 */
NAN_GETTER(RasterBand::idGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int id = band->this_->GetBand();

	if(id == 0) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	} else {
		info.GetReturnValue().Set(Nan::New<Integer>(id));
		return;
	}
}

/**
 * Name of of band.
 *
 * @readOnly
 * @attribute description
 * @type {string}
 */
NAN_GETTER(RasterBand::descriptionGetter) {
	Nan::HandleScope scope;

	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(SafeString::New(band->this_->GetDescription()));
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("x").ToLocalChecked(), Nan::New<Integer>(band->this_->GetXSize()));
	result->Set(Nan::New("y").ToLocalChecked(), Nan::New<Integer>(band->this_->GetYSize()));
	info.GetReturnValue().Set(result);
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y;
	band->this_->GetBlockSize(&x, &y);

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("x").ToLocalChecked(), Nan::New<Integer>(x));
	result->Set(Nan::New("y").ToLocalChecked(), Nan::New<Integer>(y));
	info.GetReturnValue().Set(result);
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int success = 0;
	double result = band->this_->GetMinimum(&success);
	info.GetReturnValue().Set(Nan::New<Number>(result));
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int success = 0;
	double result = band->this_->GetMaximum(&success);
	info.GetReturnValue().Set(Nan::New<Number>(result));
}

/**
 * Raster value offset.
 *
 * @attribute offset
 * @type {Number}
 */
NAN_GETTER(RasterBand::offsetGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int success = 0;
	double result = band->this_->GetOffset(&success);
	info.GetReturnValue().Set(Nan::New<Number>(result));
}

/**
 * Raster value scale.
 *
 * @attribute scale
 * @type {Number}
 */
NAN_GETTER(RasterBand::scaleGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int success = 0;
	double result = band->this_->GetScale(&success);
	info.GetReturnValue().Set(Nan::New<Number>(result));
}

/**
 * No data value for this band.
 *
 * @attribute noDataValue
 * @type {Number|null}
 */
NAN_GETTER(RasterBand::noDataValueGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int success = 0;
	double result = band->this_->GetNoDataValue(&success);

	if(success && !CPLIsNan(result)) {
		info.GetReturnValue().Set(Nan::New<Number>(result));
		return;
	} else {
		info.GetReturnValue().Set(Nan::Null());
		return;
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	const char *result = band->this_->GetUnitType();
	info.GetReturnValue().Set(SafeString::New(result));
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	GDALDataType type = band->this_->GetRasterDataType();

	if(type == GDT_Unknown) return;
	info.GetReturnValue().Set(SafeString::New(GDALGetDataTypeName(type)));
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	GDALAccess result = band->this_->GetAccess();
	info.GetReturnValue().Set(result == GA_Update ? Nan::False() : Nan::True());
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	bool result = band->this_->HasArbitraryOverviews();
	info.GetReturnValue().Set(Nan::New<Boolean>(result));
}

/**
 * List of list of category names for this raster.
 *
 * @attribute categoryNames
 * @type {Array}
 */
NAN_GETTER(RasterBand::categoryNamesGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	char ** names = band->this_->GetCategoryNames();

	Local<Array> results = Nan::New<Array>();

	if (names) {
		int i = 0;
		while (names[i]) {
			results->Set(i, Nan::New(names[i]).ToLocalChecked());
			i++;
		}
	}

	info.GetReturnValue().Set(results);
}

/**
 * Color interpretation mode ({{#crossLink "Constants (GCI)"}}see GCI constants{{/crossLink}}).
 *
 * @attribute colorInterpretation
 * @type {string}
 */
NAN_GETTER(RasterBand::colorInterpretationGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}
	GDALColorInterp interp = band->this_->GetColorInterpretation();
	if(interp == GCI_Undefined) return;
	else info.GetReturnValue().Set(SafeString::New(GDALGetColorInterpretationName(interp)));
}

NAN_SETTER(RasterBand::unitTypeSetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsString()) {
		Nan::ThrowError("Unit type must be a string");
		return;
	}
	std::string input = *Nan::Utf8String(value);
	CPLErr err = band->this_->SetUnitType(input.c_str());
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::noDataValueSetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	double input;

	if (value->IsNull() || value -> IsUndefined()){
		input = std::numeric_limits<double>::quiet_NaN();
	} else if (value->IsNumber()) {
		input = value->NumberValue();
	} else {
		Nan::ThrowError("No data value must be a number");
		return;
	}

	CPLErr err = band->this_->SetNoDataValue(input);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_SETTER(RasterBand::scaleSetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsNumber()) {
		Nan::ThrowError("Scale must be a number");
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	if (!value->IsNumber()) {
		Nan::ThrowError("Offset must be a number");
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	if(!value->IsArray()){
		Nan::ThrowError("Category names must be an array");
		return;
	}
	Local<Array> names = value.As<Array>();

	char **list = NULL;
	std::string *strlist = NULL;

	if (names->Length() > 0) {
		list = new char* [names->Length() + 1];
		strlist = new std::string [names->Length()];
		unsigned int i;
		for (i = 0; i < names->Length(); i++) {
			strlist[i] = *Nan::Utf8String(names->Get(i));
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
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	GDALColorInterp ci = GCI_Undefined;

	if (value->IsString()) {
		std::string name = *Nan::Utf8String(value);
		ci = GDALGetColorInterpretationByName(name.c_str());
	} else if(!value->IsNull() && !value->IsUndefined()) {
		Nan::ThrowError("color interpretation must be a string or undefined");
		return;
	}

	CPLErr err = band->this_->SetColorInterpretation(ci);
	if (err) {
		NODE_THROW_CPLERR(err);
	}
}

NAN_GETTER(RasterBand::uidGetter)
{
	Nan::HandleScope scope;
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(info.This());
	info.GetReturnValue().Set(Nan::New((int)band->uid));
}

} // namespace node_gdal
