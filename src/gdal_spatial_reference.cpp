
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"
#include "utils/string_list.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> SpatialReference::constructor;
ObjectCache<OGRSpatialReference, SpatialReference> SpatialReference::cache;

void SpatialReference::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(SpatialReference::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("SpatialReference"));

	NODE_SET_METHOD(lcons, "fromUserInput", fromUserInput);
	NODE_SET_METHOD(lcons, "fromWKT", fromWKT);
	NODE_SET_METHOD(lcons, "fromProj4", fromProj4);
	NODE_SET_METHOD(lcons, "fromEPSG", fromEPSG);
	NODE_SET_METHOD(lcons, "fromEPSGA", fromEPSGA);
	NODE_SET_METHOD(lcons, "fromESRI", fromESRI);
	NODE_SET_METHOD(lcons, "fromWMSAUTO", fromWMSAUTO);
	NODE_SET_METHOD(lcons, "fromXML", fromXML);
	NODE_SET_METHOD(lcons, "fromURN", fromURN);
	NODE_SET_METHOD(lcons, "fromCRSURL", fromCRSURL);
	NODE_SET_METHOD(lcons, "fromURL", fromURL);
	NODE_SET_METHOD(lcons, "fromMICoordSys", fromMICoordSys);

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toWKT", exportToWKT);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toPrettyWKT", exportToPrettyWKT);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toProj4", exportToProj4);
	NODE_SET_PROTOTYPE_METHOD(lcons, "toXML", exportToXML);

	NODE_SET_PROTOTYPE_METHOD(lcons, "clone", clone);
	NODE_SET_PROTOTYPE_METHOD(lcons, "cloneGeogCS", cloneGeogCS);
	NODE_SET_PROTOTYPE_METHOD(lcons, "setWellKnownGeogCS", setWellKnownGeogCS);
	NODE_SET_PROTOTYPE_METHOD(lcons, "morphToESRI", morphToESRI);
	NODE_SET_PROTOTYPE_METHOD(lcons, "morphFromESRI", morphFromESRI);
	NODE_SET_PROTOTYPE_METHOD(lcons, "EPSGTreatsAsLatLong", EPSGTreatsAsLatLong);
	NODE_SET_PROTOTYPE_METHOD(lcons, "EPSGTreatsAsNorthingEasting", EPSGTreatsAsNorthingEasting);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getLinearUnits", getLinearUnits);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getAngularUnits", getAngularUnits);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isGeocentric", isGeocentric);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isProjected", isProjected);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isLocal", isLocal);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isVectical", isVertical);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isCompound", isCompound);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isSameGeogCS", isSameGeogCS);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isSameVertCS", isSameVertCS);
	NODE_SET_PROTOTYPE_METHOD(lcons, "isSame", isSame);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getAuthorityName", getAuthorityName);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getAuthorityCode", getAuthorityCode);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getAttrValue", getAttrValue);
	NODE_SET_PROTOTYPE_METHOD(lcons, "autoIdentifyEPSG", autoIdentifyEPSG);
	NODE_SET_PROTOTYPE_METHOD(lcons, "validate", validate);

	target->Set(NanNew("SpatialReference"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

SpatialReference::SpatialReference(OGRSpatialReference *srs)
	: ObjectWrap(),
	  this_(srs),
	  owned_(false)
{
	LOG("Created SpatialReference [%p]", srs);
}

SpatialReference::SpatialReference()
	: ObjectWrap(),
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
	NanScope();
	SpatialReference *f;
	OGRSpatialReference *srs;
	std::string wkt("");

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		f = static_cast<SpatialReference *>(ptr);
		f->Wrap(args.This());
	} else {
		NODE_ARG_OPT_STR(0, "wkt", wkt);
		//sets reference count to one
		srs = new OGRSpatialReference(wkt.empty() ? 0 : wkt.c_str());
		if (!wkt.empty()) {
			char* wkt_c = (char*) wkt.c_str();
			int err = srs->importFromWkt(&wkt_c);
			if (err) {
				NODE_THROW_OGRERR(err);
				NanReturnUndefined();
			}
		}
		f = new SpatialReference(srs);
		f->owned_ = true;
		f->Wrap(args.This());

		cache.add(srs, args.This());
	}

	NanReturnValue(args.This());
}

Handle<Value> SpatialReference::New(OGRSpatialReference *srs)
{
	NanEscapableScope();
	return NanEscapeScope(SpatialReference::New(srs, false));
}

Handle<Value> SpatialReference::New(OGRSpatialReference *raw, bool owned)
{
	NanEscapableScope();

	if (!raw) {
		return NanEscapeScope(NanNull());
	}
	if (cache.has(raw)) {
		return NanEscapeScope(NanNew(cache.get(raw)));
	}

	//make a copy of spatialreference owned by a layer, feature, etc
	// + no need to track when a layer is destroyed
	// + no need to throw errors when a method trys to modify an owned read-only srs
	// - is slower

	OGRSpatialReference* cloned_srs = raw->Clone();

	SpatialReference *wrapped = new SpatialReference(cloned_srs);
	wrapped->owned_ = true;
	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(SpatialReference::constructor)->GetFunction()->NewInstance(1, &ext);

	cache.add(cloned_srs, raw, obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(SpatialReference::toString)
{
	NanScope();
	NanReturnValue(NanNew("SpatialReference"));
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
 * Check if geocentric coordinate system.
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
	NanScope();
	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	NanReturnValue(SpatialReference::New(srs->this_->Clone()));
}

/**
 * Make a duplicate of the GEOGCS node of this OGRSpatialReference object.
 *
 * @method cloneGeogCS
 * @return {gdal.SpatialReference}
 */
NAN_METHOD(SpatialReference::cloneGeogCS)
{
	NanScope();
	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	NanReturnValue(SpatialReference::New(srs->this_->CloneGeogCS()));
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
	NanScope();

	std::string key = "";
	NODE_ARG_STR(0, "target key", key);

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	NanReturnValue(SafeString::New(srs->this_->GetAuthorityName(key.c_str())));
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
	NanScope();

	std::string key = "";
	NODE_ARG_STR(0, "target key", key);

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	NanReturnValue(SafeString::New(srs->this_->GetAuthorityCode(key.c_str())));
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
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	char* str;
	Handle<Value> result;

	int err = srs->this_->exportToWkt(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	result = SafeString::New(str);
	CPLFree(str);

	NanReturnValue(result);
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
	NanScope();

	int simplify = 0;
	NODE_ARG_BOOL_OPT(0, "simplify", simplify);

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	char* str;
	Handle<Value> result;

	int err = srs->this_->exportToPrettyWkt(&str, simplify);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	result = SafeString::New(str);
	CPLFree(str);

	NanReturnValue(result);
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
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	char* str;
	Handle<Value> result;

	int err = srs->this_->exportToProj4(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	if(str){
		result = NanNew(CPLString(str).Trim().c_str());
	} else {
		result = NanNull();
	}
	CPLFree(str);

	NanReturnValue(result);
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
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	char* str;
	Handle<Value> result;

	int err = srs->this_->exportToXML(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	result = SafeString::New(str);
	CPLFree(str);

	NanReturnValue(result);
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
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	std::string node_name("");
	int child = 0;
	NODE_ARG_STR(0, "node name", node_name);
	NODE_ARG_INT_OPT(1, "child", child);
	NanReturnValue(SafeString::New(srs->this_->GetAttrValue(node_name.c_str(), child)));
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
	NanScope();

	std::string wkt("");
	NODE_ARG_STR(0, "wkt", wkt);
	char* str = (char*) wkt.c_str();

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromWkt(&str);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromProj4(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromWMSAUTO(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "xml", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromXML(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromURN(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "url", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromCRSURL(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "url", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromUrl(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromMICoordSys(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	std::string input("");
	NODE_ARG_STR(0, "input", input);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->SetFromUserInput(input.c_str());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	int epsg;
	NODE_ARG_INT(0, "epsg", epsg);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromEPSG(epsg);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	int epsg;
	NODE_ARG_INT(0, "epsg", epsg);

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromEPSGA(epsg);
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
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
	NanScope();

	StringList list;

	if(args.Length() < 1) {
		NanThrowError("input string list must be provided");
		NanReturnUndefined();
	}

	if(list.parse(args[0])) {
		NanReturnUndefined(); //error parsing string list
	}

	OGRSpatialReference *srs = new OGRSpatialReference();
	int err = srs->importFromESRI(list.get());
	if (err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnValue(SpatialReference::New(srs, true));
}

/**
 * Fetch linear geographic coordinate system units.
 *
 * @method getLinearUnits
 * @return {Object} An object containing `value` and `unit` properties.
 */
NAN_METHOD(SpatialReference::getLinearUnits)
{
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	char* unit_name;
	double units = srs->this_->GetLinearUnits(&unit_name);

	Handle<Object> result = NanNew<Object>();
	result->Set(NanNew("value"), NanNew<Number>(units));
	result->Set(NanNew("units"), SafeString::New(unit_name));

	NanReturnValue(result);
}

/**
 * Fetch angular geographic coordinate system units.
 *
 * @method getAngularUnits
 * @return {Object} An object containing `value` and `unit` properties.
 */
NAN_METHOD(SpatialReference::getAngularUnits)
{
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	char* unit_name;
	double units = srs->this_->GetAngularUnits(&unit_name);

	Handle<Object> result = NanNew<Object>();
	result->Set(NanNew("value"), NanNew<Number>(units));
	result->Set(NanNew("units"), SafeString::New(unit_name));

	NanReturnValue(result);
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
	NanScope();

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	OGRErr err = srs->this_->Validate();

	if(err == OGRERR_NONE) {
		NanReturnNull();
	}
	if(err == OGRERR_CORRUPT_DATA) {
		NanReturnValue(NanNew("corrupt"));
	}
	if(err == OGRERR_UNSUPPORTED_SRS) {
		NanReturnValue(NanNew("unsupported"));
	}

	NODE_THROW_OGRERR(err);
	NanReturnUndefined();
}


} // namespace node_gdal