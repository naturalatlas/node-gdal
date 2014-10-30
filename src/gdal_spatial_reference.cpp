
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> SpatialReference::constructor;
ObjectCache<OGRSpatialReference> SpatialReference::cache;

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
	NODE_SET_PROTOTYPE_METHOD(lcons, "cloneGeogCS", clone);
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

NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, setWellKnownGeogCS, SetWellKnownGeogCS, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphToESRI, morphToESRI);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphFromESRI, morphFromESRI);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsLatLong, Boolean, EPSGTreatsAsLatLong);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsNorthingEasting, Boolean, EPSGTreatsAsNorthingEasting);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isGeocentric, Boolean, IsGeocentric);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isProjected, Boolean, IsProjected);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isLocal, Boolean, IsLocal);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isVertical, Boolean, IsVertical);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isCompound, Boolean, IsCompound);
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameGeogCS, Boolean, IsSameGeogCS, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameVertCS, Boolean, IsSameVertCS, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSame, Boolean, IsSame, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, autoIdentifyEPSG, AutoIdentifyEPSG);

NAN_METHOD(SpatialReference::clone)
{
	NanScope();
	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	NanReturnValue(SpatialReference::New(srs->this_->Clone()));
}

NAN_METHOD(SpatialReference::cloneGeogCS)
{
	NanScope();
	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
	NanReturnValue(SpatialReference::New(srs->this_->CloneGeogCS()));
}

NAN_METHOD(SpatialReference::getAuthorityName)
{
	NanScope();

	std::string key = "";
	NODE_ARG_STR(0, "target key", key);

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	NanReturnValue(SafeString::New(srs->this_->GetAuthorityName(key.c_str())));
}

NAN_METHOD(SpatialReference::getAuthorityCode)
{
	NanScope();

	std::string key = "";
	NODE_ARG_STR(0, "target key", key);

	SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());

	NanReturnValue(SafeString::New(srs->this_->GetAuthorityCode(key.c_str())));
}

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

} // namespace node_gdal