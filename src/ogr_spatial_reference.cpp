
#include "ogr_common.hpp"
#include "ogr_spatial_reference.hpp"

using namespace node_ogr;

Persistent<FunctionTemplate> SpatialReference::constructor;
ObjectCache<OGRSpatialReference*> SpatialReference::cache;

void SpatialReference::Initialize(Handle<Object> target) {
    HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(SpatialReference::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("SpatialReference"));

    NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
    NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
    NODE_SET_PROTOTYPE_METHOD(constructor, "cloneGeogCS", clone);
    NODE_SET_PROTOTYPE_METHOD(constructor, "exportToWKT", exportToWKT);
    NODE_SET_PROTOTYPE_METHOD(constructor, "exportToPrettyWKT", exportToPrettyWKT);
    NODE_SET_PROTOTYPE_METHOD(constructor, "exportToProj4", exportToProj4);
    NODE_SET_PROTOTYPE_METHOD(constructor, "exportToXML", exportToXML);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromWKT", importFromWKT);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromProj4", importFromProj4);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromEPSG", importFromEPSG);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromEPSGA", importFromEPSGA);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromWMSAUTO", importFromWMSAUTO);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromXML", importFromXML);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromURN", importFromURN);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromCRSURL", importFromCRSURL);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromURL", importFromURL);
    NODE_SET_PROTOTYPE_METHOD(constructor, "importFromMICoordSys", importFromMICoordSys);
    NODE_SET_PROTOTYPE_METHOD(constructor, "setWellKnownGeogCS", setWellKnownGeogCS);
    NODE_SET_PROTOTYPE_METHOD(constructor, "setFromUserInput", setFromUserInput);
    NODE_SET_PROTOTYPE_METHOD(constructor, "morphToESRI", morphToESRI);
    NODE_SET_PROTOTYPE_METHOD(constructor, "morphFromESRI", morphFromESRI);
    NODE_SET_PROTOTYPE_METHOD(constructor, "EPSGTreatsAsLatLong", EPSGTreatsAsLatLong);
    NODE_SET_PROTOTYPE_METHOD(constructor, "EPSGTreatsAsNorthingEasting", EPSGTreatsAsNorthingEasting);
    NODE_SET_PROTOTYPE_METHOD(constructor, "getLinearUnits", getLinearUnits);
    NODE_SET_PROTOTYPE_METHOD(constructor, "getAngularUnits", getAngularUnits);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isGeocentric", isGeocentric);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isProjected", isProjected);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isLocal", isLocal);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isVectical", isVertical);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isCompound", isCompound);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isSameGeogCS", isSameGeogCS);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isSameVertCS", isSameVertCS);
    NODE_SET_PROTOTYPE_METHOD(constructor, "isSame", isSame);
    NODE_SET_PROTOTYPE_METHOD(constructor, "getAuthorityName", getAuthorityName);
    NODE_SET_PROTOTYPE_METHOD(constructor, "getAuthorityCode", getAuthorityCode);
    NODE_SET_PROTOTYPE_METHOD(constructor, "getAttrValue", getAttrValue);
    NODE_SET_PROTOTYPE_METHOD(constructor, "autoIdentifyEPSG", autoIdentifyEPSG);

    target->Set(String::NewSymbol("SpatialReference"), constructor->GetFunction());
}

SpatialReference::SpatialReference(OGRSpatialReference *srs)
: ObjectWrap(),
  this_(srs),
  owned_(false)
{
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

void SpatialReference::dispose(){
  if(owned_ && this_){
    //Decrements the reference count by one, and destroy if zero.
    this_->Release();
  }
  this_ = NULL;
}

Handle<Value> SpatialReference::New(const Arguments& args)
{
  HandleScope scope;
  SpatialReference *f;
  OGRSpatialReference *srs; 
  std::string wkt("");

  if (!args.IsConstructCall())
    return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

  if (args[0]->IsExternal()) {
    Local<External> ext = Local<External>::Cast(args[0]);
    void* ptr = ext->Value();
    f = static_cast<SpatialReference *>(ptr);
    f->Wrap(args.This());
  }else{
    NODE_ARG_OPT_STR(0, "wkt", wkt);
    //sets reference count to one
    srs = new OGRSpatialReference(wkt.empty() ? 0 : wkt.c_str());
    if(!wkt.empty()) {
      char* wkt_c = (char*) wkt.c_str();
      int err = srs->importFromWkt(&wkt_c);
      if(err) return NODE_THROW_OGRERR(err);
    }
    f = new SpatialReference(srs);
    f->owned_ = true;
    f->Wrap(args.This());
    
    cache.add(srs, args.This());
  }

  return args.This();
}

Handle<Value> SpatialReference::New(OGRSpatialReference *srs) {
  HandleScope scope;
  return scope.Close(SpatialReference::New(srs, false));
}

Handle<Value> SpatialReference::New(OGRSpatialReference *raw, bool owned) {
  HandleScope scope;

  if(!raw) return v8::Null();
  if(cache.has(raw)) return cache.get(raw);

  //make a copy of spatialreference owned by a layer, feature, etc
  // + no need to track when a layer is destroyed
  // + no need to throw errors when a method trys to modify an owned read-only srs
  // - is slower

  OGRSpatialReference* cloned_srs = raw->Clone();

  SpatialReference *wrapped = new SpatialReference(cloned_srs);
  wrapped->owned_ = true;
  v8::Handle<v8::Value> ext = v8::External::New(wrapped);
  v8::Handle<v8::Object> obj = SpatialReference::constructor->GetFunction()->NewInstance(1, &ext);

  cache.add(cloned_srs, obj);
  cache.addAlias(cloned_srs, raw); //fetch the srs copy from cache, rather than recloning 

  return scope.Close(obj);
}

Handle<Value> SpatialReference::toString(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(String::New("SpatialReference"));
}


NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, clone, SpatialReference, Clone);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, cloneGeogCS, SpatialReference, CloneGeogCS);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromProj4, importFromProj4, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(SpatialReference, importFromEPSG, importFromEPSG, "epsg");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(SpatialReference, importFromEPSGA, importFromEPSGA, "epsg");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromWMSAUTO, importFromWMSAUTO, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromXML, importFromXML, "xml");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromURN, importFromURN, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromCRSURL, importFromCRSURL, "url");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromURL, importFromUrl, "url");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, importFromMICoordSys, importFromMICoordSys, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, setWellKnownGeogCS, SetWellKnownGeogCS, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_STRING_PARAM(SpatialReference, setFromUserInput, SetFromUserInput, "input");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphToESRI, morphToESRI);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, morphFromESRI, morphFromESRI);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsLatLong, Boolean, EPSGTreatsAsLatLong);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, EPSGTreatsAsNorthingEasting, Boolean, EPSGTreatsAsNorthingEasting);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, getLinearUnits, Number, GetLinearUnits);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, getAngularUnits, Number, GetAngularUnits);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isGeocentric, Boolean, IsGeocentric);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isProjected, Boolean, IsProjected);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isLocal, Boolean, IsLocal);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isVertical, Boolean, IsVertical);
NODE_WRAPPED_METHOD_WITH_RESULT(SpatialReference, isCompound, Boolean, IsCompound);
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameGeogCS, Boolean, IsSameGeogCS, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSameVertCS, Boolean, IsSameVertCS, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(SpatialReference, isSame, Boolean, IsSame, SpatialReference, "srs");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(SpatialReference, autoIdentifyEPSG, AutoIdentifyEPSG);
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(SpatialReference, getAuthorityName, SafeString, GetAuthorityName, "target key");
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(SpatialReference, getAuthorityCode, SafeString, GetAuthorityCode, "target key");

Handle<Value> SpatialReference::exportToWKT(const Arguments& args)
{
  HandleScope scope;

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  char* str;
  Handle<Value> result;

  int err = srs->this_->exportToWkt(&str);
  if (err) return NODE_THROW_OGRERR(err);
  result = SafeString::New(str);
  CPLFree(str);

  return scope.Close(result);
}

Handle<Value> SpatialReference::exportToPrettyWKT(const Arguments& args)
{
  HandleScope scope;

  int simplify = 0;
  NODE_ARG_BOOL_OPT(0, "simplify", simplify);

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  char* str;
  Handle<Value> result;

  int err = srs->this_->exportToPrettyWkt(&str, simplify);
  if (err) return NODE_THROW_OGRERR(err);
  result = SafeString::New(str);
  CPLFree(str);

  return scope.Close(result);
}


Handle<Value> SpatialReference::exportToProj4(const Arguments& args)
{
  HandleScope scope;

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  char* str;
  Handle<Value> result;

  int err = srs->this_->exportToProj4(&str);
  if (err) return NODE_THROW_OGRERR(err);
  result = SafeString::New(str);
  CPLFree(str);

  return scope.Close(result);
}

Handle<Value> SpatialReference::exportToXML(const Arguments& args)
{
  HandleScope scope;

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  char* str;
  Handle<Value> result;

  int err = srs->this_->exportToXML(&str);
  if (err) return NODE_THROW_OGRERR(err);
  result = SafeString::New(str);
  CPLFree(str);

  return scope.Close(result);
}

Handle<Value> SpatialReference::importFromWKT(const Arguments& args)
{
  HandleScope scope;

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  std::string wkt("");
  NODE_ARG_STR(0, "wkt", wkt);
  char* str = (char*) wkt.c_str();

  int err = srs->this_->importFromWkt(&str);
  if (err) return NODE_THROW_OGRERR(err);

  return Undefined();
}

Handle<Value> SpatialReference::getAttrValue(const Arguments& args)
{
  HandleScope scope;

  SpatialReference *srs = ObjectWrap::Unwrap<SpatialReference>(args.This());
  std::string node_name("");
  int child = 0;
  NODE_ARG_STR(0, "node name", node_name);
  NODE_ARG_INT_OPT(1, "child", child);
  return scope.Close(SafeString::New(srs->this_->GetAttrValue(node_name.c_str(), child)));
}
