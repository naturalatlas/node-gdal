
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"
#include "utils/string_list.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> SpatialReference::constructor;
ObjectCache<OGRSpatialReference, SpatialReference> SpatialReference::cache;

void SpatialReference::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(SpatialReference::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("SpatialReference").ToLocalChecked());

	Nan::SetMethod(lcons, "fromUserInput", fromUserInput);
	Nan::SetMethod(lcons, "fromWKT", fromWKT);
	Nan::SetMethod(lcons, "fromProj4", fromProj4);
	Nan::SetMethod(lcons, "fromEPSG", fromEPSG);
	Nan::SetMethod(lcons, "fromEPSGA", fromEPSGA);
	Nan::SetMethod(lcons, "fromESRI", fromESRI);
	Nan::SetMethod(lcons, "fromWMSAUTO", fromWMSAUTO);
	Nan::SetMethod(lcons, "fromXML", fromXML);
	Nan::SetMethod(lcons, "fromURN", fromURN);
	Nan::SetMethod(lcons, "fromCRSURL", fromCRSURL);
	Nan::SetMethod(lcons, "fromURL", fromURL);
	Nan::SetMethod(lcons, "fromMICoordSys", fromMICoordSys);

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "toWKT", exportToWKT);
	Nan::SetPrototypeMethod(lcons, "toPrettyWKT", exportToPrettyWKT);
	Nan::SetPrototypeMethod(lcons, "toProj4", exportToProj4);
	Nan::SetPrototypeMethod(lcons, "toXML", exportToXML);

	Nan::SetPrototypeMethod(lcons, "clone", clone);
	Nan::SetPrototypeMethod(lcons, "cloneGeogCS", cloneGeogCS);
	Nan::SetPrototypeMethod(lcons, "setWellKnownGeogCS", setWellKnownGeogCS);
	Nan::SetPrototypeMethod(lcons, "morphToESRI", morphToESRI);
	Nan::SetPrototypeMethod(lcons, "morphFromESRI", morphFromESRI);
	Nan::SetPrototypeMethod(lcons, "EPSGTreatsAsLatLong", EPSGTreatsAsLatLong);
	Nan::SetPrototypeMethod(lcons, "EPSGTreatsAsNorthingEasting", EPSGTreatsAsNorthingEasting);
	Nan::SetPrototypeMethod(lcons, "getLinearUnits", getLinearUnits);
	Nan::SetPrototypeMethod(lcons, "getAngularUnits", getAngularUnits);
	Nan::SetPrototypeMethod(lcons, "isGeographic", isGeographic);
	Nan::SetPrototypeMethod(lcons, "isGeocentric", isGeocentric);
	Nan::SetPrototypeMethod(lcons, "isProjected", isProjected);
	Nan::SetPrototypeMethod(lcons, "isLocal", isLocal);
	Nan::SetPrototypeMethod(lcons, "isVectical", isVertical);
	Nan::SetPrototypeMethod(lcons, "isCompound", isCompound);
	Nan::SetPrototypeMethod(lcons, "isSameGeogCS", isSameGeogCS);
	Nan::SetPrototypeMethod(lcons, "isSameVertCS", isSameVertCS);
	Nan::SetPrototypeMethod(lcons, "isSame", isSame);
	Nan::SetPrototypeMethod(lcons, "getAuthorityName", getAuthorityName);
	Nan::SetPrototypeMethod(lcons, "getAuthorityCode", getAuthorityCode);
	Nan::SetPrototypeMethod(lcons, "getAttrValue", getAttrValue);
	Nan::SetPrototypeMethod(lcons, "autoIdentifyEPSG", autoIdentifyEPSG);
	Nan::SetPrototypeMethod(lcons, "validate", validate);

	target->Set(Nan::New("SpatialReference").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

SpatialReference::SpatialReference(OGRSpatialReference *srs)
	: Nan::ObjectWrap(),
	  this_(srs),
	  owned_(false)
{
	LOG("Created SpatialReference [%p]", srs);
}

SpatialReference::SpatialReference()
	: Nan::ObjectWrap(),
	  this_(0),
	  owned_(false)
{
}

SpatialReference::~SpatialReference()
{
	dispose();
}

void SpatialReference::dispose()
{
	if (this_) {
		LOG("Disposing SpatialReference [%p] (%s)", this_, owned_ ? "owned" : "unowned");
		cache.erase(this_);
		if (owned_) {
			//Decrements the reference count by one, and destroy if zero.
			this_->Release();
		}
		LOG("Disposed SpatialReference [%p]", this_);
		this_ = NULL;
	}
}

/**
 * This class respresents a OpenGIS Spatial Reference System, and contains methods
 * for converting between this object organization and well known text (WKT)
 * format.
 *
 * @constructor
 * @class gdal.SpatialReference
 * @param {String} [wkt]
 */
NAN_METHOD(SpatialReference::New)
{
	Nan::HandleScope scope;
	SpatialReference *f;
	OGRSpatialReference *srs;
	std::string wkt("");

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<SpatialReference *>(ptr);
		f->Wrap(info.This());
	} else {
		NODE_ARG_OPT_STR(0, "wkt", wkt);
		//sets reference count to one
		srs = new OGRSpatialReference(wkt.empty() ? 0 : wkt.c_str());
		if (!wkt.empty()) {
			char* wkt_c = (char*) wkt.c_str();
			int err = srs->importFromWkt(&wkt_c);
			if (err) {
				NODE_THROW_OGRERR(err);
				return;
			}
		}
		f = new SpatialReference(srs);
		f->owned_ = true;
		f->Wrap(info.This());

		cache.add(srs, info.This());
	}

	info.GetReturnValue().Set(info.This());
}

Local<Value> SpatialReference::New(OGRSpatialReference *srs)
{
	Nan::EscapableHandleScope scope;
	return scope.Escape(SpatialReference::New(srs, false));
}

Local<Value> SpatialReference::New(OGRSpatialReference *raw, bool owned)
{
	Nan::EscapableHandleScope scope;

	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	if (cache.has(raw)) {
		return scope.Escape(cache.get(raw));
	}

	//make a copy of spatialreference owned by a layer, feature, etc
	// + no need to track when a layer is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only srs
	// - is slower

	OGRSpatialReference* cloned_srs = raw->Clone();

	SpatialReference *wrapped = new SpatialReference(cloned_srs);
	wrapped->owned_ = true;
	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::NewInstance(Nan::New(SpatialReference::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	cache.add(cloned_srs, raw, obj);

	return scope.Escape(obj);
}

NAN_METHOD(SpatialReference::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("SpatialReference").ToLocalChecked());
}

/**
 * Set a GeogCS based on well known name.
 *
 * @method setWellKnownGeogCS
 * @param {String} name
 */
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, setWellKnownGeogCS, SetWellKnownGeogCS, "input");

/**
 * Convert in place to ESRI WKT format.
 *
 * @throws Error
 * @method morphToESRI
 */
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphToESRI, morphToESRI);

/**
 * Convert in place from ESRI WKT format.
 *
 * @throws Error
 * @method morphFromESRI
 */
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphFromESRI, morphFromESRI);

/**
 * This method returns `true` if EPSG feels this geographic coordinate system
 * should be treated as having lat/long coordinate ordering.
 *
 * Currently this returns `true` for all geographic coordinate systems with an
 * EPSG code set, and AXIS values set defining it as lat, long. Note that
 * coordinate systems with an EPSG code and no axis settings will be assumed
 * to not be lat/long.
 *
 * `false` will be returned for all coordinate systems that are not geographic,
 * or that do not have an EPSG code set.
 *
 * @method EPSGTreatsAsLatLong
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsLatLong, Boolean, EPSGTreatsAsLatLong);

/**
 * This method returns `true` if EPSG feels this projected coordinate system should
 * be treated as having northing/easting coordinate ordering.
 *
 * @method EPSGTreatsAsNorthingEasting
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsNorthingEasting, Boolean, EPSGTreatsAsNorthingEasting);

/**
 * Check if geocentric coordinate system.
 *
 * @method isGeocentric
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isGeocentric, Boolean, IsGeocentric);

/**
 * Check if geographic coordinate system.
 *
 * @method isGeographic
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isGeographic, Boolean, IsGeographic);

/**
 * Check if projected coordinate system.
 *
 * @method isProjected
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isProjected, Boolean, IsProjected);

/**
 * Check if local coordinate system.
 *
 * @method isLocal
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isLocal, Boolean, IsLocal);

/**
 * Check if vertical coordinate system.
 *
 * @method isVertical
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isVertical, Boolean, IsVertical);

/**
 * Check if compound coordinate system.
 *
 * @method isCompound
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isCompound, Boolean, IsCompound);

/**
 * Do the GeogCS'es match?
 *
 * @method isSameGeogCS
 * @param {gdal.SpatialReference} srs
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameGeogCS, Boolean, IsSameGeogCS, SpatialReference, "srs");

/**
 * Do the VertCS'es match?
 *
 * @method isSameVertCS
 * @param {gdal.SpatialReference} srs
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameVertCS, Boolean, IsSameVertCS, SpatialReference, "srs");

/**
 * Do these two spatial references describe the same system?
 *
 * @method isSame
 * @param {gdal.SpatialReference} srs
 * @return {Boolean}
 */
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSame, Boolean, IsSame, SpatialReference, "srs");

/**
 * Set EPSG authority info if possible.
 *
 * @throws Error
 * @method autoIdentifyEPSG
 */
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, autoIdentifyEPSG, AutoIdentifyEPSG);

/**
 * Clones the spatial reference.
 *
 * @method clone
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::clone)
{
	Nan::HandleScope scope;
	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	info.GetReturnValue().Set(SpatialReference::New(srs->this_->Clone()));
}

/**
 * Make a duplicate of the GEOGCS node of this OGRSpatialReference object.
 *
 * @method cloneGeogCS
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::cloneGeogCS)
{
	Nan::HandleScope scope;
	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	info.GetReturnValue().Set(SpatialReference::New(srs->this_->CloneGeogCS()));
}

/**
 * Get the authority name for a node. The most common authority is "EPSG".
 *
 * @method getAuthorityName
 * @param {string} target_key The partial or complete path to the node to get an authority from. ie. `"PROJCS"`, `"GEOGCS"`, "`GEOGCS|UNIT"` or `null` to search for an authority node on the root element.
 * @return {string}
 */
NAN_METHOD(SpatialReference::getAuthorityName)
{
	Nan::HandleScope scope;

	std::string key = "";
	NODE_ARG_OPT_STR(0, "target key", key);

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());

	info.GetReturnValue().Set(SafeString::New(srs->this_->GetAuthorityName(key.length() ? key.c_str() : NULL)));
}

/**
 * Get the authority code for a node.
 *
 * @method getAuthorityCode
 * @param {string} target_key The partial or complete path to the node to get an authority from. ie. `"PROJCS"`, `"GEOGCS"`, "`GEOGCS|UNIT"` or `null` to search for an authority node on the root element.
 * @return {string}
 */
NAN_METHOD(SpatialReference::getAuthorityCode)
{
	Nan::HandleScope scope;

	std::string key = "";
	NODE_ARG_OPT_STR(0, "target key", key);

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());

	info.GetReturnValue().Set(SafeString::New(srs->this_->GetAuthorityCode(key.length() ? key.c_str() : NULL)));
}

/**
 * Convert this SRS into WKT format.
 *
 * @throws Error
 * @method toWKT
 * @return {string}
 */
NAN_METHOD(SpatialReference::exportToWKT)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	char* str;
	Local<Value> result;

	int err = srs->this_->exportToWkt(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	result = SafeString::New(str);
	CPLFree(str);

	info.GetReturnValue().Set(result);
}

/**
 * Convert this SRS into a a nicely formatted WKT string for display to a person.
 *
 * @throws Error
 * @method toPrettyWKT
 * @param {Boolean} [simplify=false]
 * @return {string}
 */
NAN_METHOD(SpatialReference::exportToPrettyWKT)
{
	Nan::HandleScope scope;

	int simplify = 0;
	NODE_ARG_BOOL_OPT(0, "simplify", simplify);

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	char* str;
	Local<Value> result;

	int err = srs->this_->exportToPrettyWkt(&str, simplify);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	result = SafeString::New(str);
	CPLFree(str);

	info.GetReturnValue().Set(result);
}

/**
 * Export coordinate system in PROJ.4 format.
 *
 * @throws Error
 * @method toProj4
 * @return {string}
 */
NAN_METHOD(SpatialReference::exportToProj4)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	char* str;
	Local<Value> result;

	int err = srs->this_->exportToProj4(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	if(str){
		result = Nan::New(CPLString(str).Trim().c_str()).ToLocalChecked();
	} else {
		result = Nan::Null();
	}
	CPLFree(str);

	info.GetReturnValue().Set(result);
}

/**
 * Export coordinate system in XML format.
 *
 * @throws Error
 * @method toXML
 * @return {string}
 */
NAN_METHOD(SpatialReference::exportToXML)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	char* str;
	Local<Value> result;

	int err = srs->this_->exportToXML(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}
	result = SafeString::New(str);
	CPLFree(str);

	info.GetReturnValue().Set(result);
}

/**
 * Fetch indicated attribute of named node.
 *
 * @method getAttrValue
 * @param {String} node_name
 * @param {Integer} [attr_index=0]
 * @return {string}
 */
NAN_METHOD(SpatialReference::getAttrValue)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());
	std::string node_name("");
	int child = 0;
	NODE_ARG_STR(0, "node name", node_name);
	NODE_ARG_INT_OPT(1, "child", child);
	info.GetReturnValue().Set(SafeString::New(srs->this_->GetAttrValue(node_name.c_str(), child)));
}

/**
 * Creates a spatial reference from a WKT string.
 *
 * @static
 * @throws Error
 * @method fromWKT
 * @param {String} wkt
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromWKT)
{
	Nan::HandleScope scope;

	std::string wkt("");
	NODE_ARG_STR(0, "wkt", wkt);
	char* str = (char*) wkt.c_str();

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromWkt(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Creates a spatial reference from a Proj.4 string.
 *
 * @static
 * @throws Error
 * @method fromProj4
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromProj4)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromProj4(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Creates a spatial reference from a WMSAUTO string.
 *
 * Note that the WMS 1.3 specification does not include the units code, while
 * apparently earlier specs do. GDAL tries to guess around this.
 *
 * @example
 * ```
 * var wms = 'AUTO:42001,99,8888';
 * var ref = gdal.SpatialReference.fromWMSAUTO(wms);```
 *
 * @static
 * @throws Error
 * @method fromWMSAUTO
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromWMSAUTO)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromWMSAUTO(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Import coordinate system from XML format (GML only currently).
 *
 * @static
 * @throws Error
 * @method fromXML
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromXML)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "xml", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromXML(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from OGC URN.
 *
 * The OGC URN should be prefixed with "urn:ogc:def:crs:" per recommendation
 * paper 06-023r1. Currently EPSG and OGC authority values are supported,
 * including OGC auto codes, but not including CRS1 or CRS88 (NAVD88).
 *
 * @static
 * @throws Error
 * @method fromURN
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromURN)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromURN(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from OGC URL.
 *
 * The OGC URL should be prefixed with "http://opengis.net/def/crs" per best
 * practice paper 11-135. Currently EPSG and OGC authority values are supported,
 * including OGC auto codes, but not including CRS1 or CRS88 (NAVD88).
 *
 * @static
 * @throws Error
 * @method fromCRSURL
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromCRSURL)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "url", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromCRSURL(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize spatial reference from a URL.
 *
 * This method will download the spatial reference from the given URL.
 *
 * @static
 * @throws Error
 * @method fromURL
 * @param {String} url
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromURL)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "url", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromUrl(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from a Mapinfo style CoordSys definition.
 *
 * @static
 * @throws Error
 * @method fromMICoordSys
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromMICoordSys)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromMICoordSys(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from an arbitrary spatial reference string.
 *
 * This method will examine the provided input, and try to deduce the format,
 * and then use it to initialize the spatial reference system.
 *
 * @static
 * @throws Error
 * @method fromUserInput
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromUserInput)
{
	Nan::HandleScope scope;

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->SetFromUserInput(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from EPSG GCS or PCS code.
 *
 * @example
 * ```
 * var ref = gdal.SpatialReference.fromEPSGA(4326);```
 *
 * @static
 * @throws Error
 * @method fromEPSG
 * @param {String} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromEPSG)
{
	Nan::HandleScope scope;

	int epsg;
	NODE_ARG_INT(0, "epsg", epsg);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromEPSG(epsg);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Initialize from EPSG GCS or PCS code.
 *
 * This method is similar to `fromEPSG()` except that EPSG preferred axis ordering
 * *will* be applied for geographic and projected coordinate systems. EPSG normally
 * defines geographic coordinate systems to use lat/long, and also there are also
 * a few projected coordinate systems that use northing/easting order contrary
 * to typical GIS use).
 *
 * @example
 * ```
 * var ref = gdal.SpatialReference.fromEPSGA(26910);```
 *
 * @static
 * @throws Error
 * @method fromEPSGA
 * @param {Integer} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromEPSGA)
{
	Nan::HandleScope scope;

	int epsg;
	NODE_ARG_INT(0, "epsg", epsg);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromEPSGA(epsg);
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Import coordinate system from ESRI .prj format(s).
 *
 * This function will read the text loaded from an ESRI .prj file, and translate it into an OGRSpatialReference definition. This should support many (but by no means all) old style (Arc/Info 7.x) .prj files, as well as the newer pseudo-OGC WKT .prj files. Note that new style .prj files are in OGC WKT format, but require some manipulation to correct datum names, and units on some projection parameters. This is addressed within importFromESRI() by an automatical call to morphFromESRI().
 *
 * Currently only GEOGRAPHIC, UTM, STATEPLANE, GREATBRITIAN_GRID, ALBERS, EQUIDISTANT_CONIC, TRANSVERSE (mercator), POLAR, MERCATOR and POLYCONIC projections are supported from old style files.
 *
 * @static
 * @throws Error
 * @method fromESRI
 * @param {string[]} input
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::fromESRI)
{
	Nan::HandleScope scope;

	StringList list;

	if(info.Length() < 1) {
		Nan::ThrowError("input string list must be provided");
		return;
	}

	if(list.parse(info[0])) {
		return; //error parsing string list
	}

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromESRI(list.get());
	if (err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	info.GetReturnValue().Set(SpatialReference::New(srs, true));
}

/**
 * Fetch linear geographic coordinate system units.
 *
 * @method getLinearUnits
 * @return {Object} An object containing `value` and `unit` properties.
 */
NAN_METHOD(SpatialReference::getLinearUnits)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());

	char* unit_name;
	double units = srs->this_->GetLinearUnits(&unit_name);

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("value").ToLocalChecked(), Nan::New<Number>(units));
	result->Set(Nan::New("units").ToLocalChecked(), SafeString::New(unit_name));

	info.GetReturnValue().Set(result);
}

/**
 * Fetch angular geographic coordinate system units.
 *
 * @method getAngularUnits
 * @return {Object} An object containing `value` and `unit` properties.
 */
NAN_METHOD(SpatialReference::getAngularUnits)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());

	char* unit_name;
	double units = srs->this_->GetAngularUnits(&unit_name);

	Local<Object> result = Nan::New<Object>();
	result->Set(Nan::New("value").ToLocalChecked(), Nan::New<Number>(units));
	result->Set(Nan::New("units").ToLocalChecked(), SafeString::New(unit_name));

	info.GetReturnValue().Set(result);
}

/**
 * Validate SRS tokens.
 *
 * This method attempts to verify that the spatial reference system is well formed, and consists of known tokens. The validation is not comprehensive.
 *
 * @method validate
 * @return {string|null} `"corrupt"`, '"unsupported"', `null` (if fine)
 */
NAN_METHOD(SpatialReference::validate)
{
	Nan::HandleScope scope;

	SpatialReference *srs = Nan::ObjectWrap::Unwrap<SpatialReference>(info.This());

	OGRErr err = srs->this_->Validate();

	if(err == OGRERR_NONE) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}
	if(err == OGRERR_CORRUPT_DATA) {
		info.GetReturnValue().Set(Nan::New("corrupt").ToLocalChecked());
		return;
	}
	if(err == OGRERR_UNSUPPORTED_SRS) {
		info.GetReturnValue().Set(Nan::New("unsupported").ToLocalChecked());
		return;
	}

	NODE_THROW_OGRERR(err);
	return;
}


} // namespace node_gdal
