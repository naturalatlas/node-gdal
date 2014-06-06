#ifndef __NODE_OGR_SPATIALREFERENCE_H__
#define __NODE_OGR_SPATIALREFERENCE_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>
#include "obj_cache.hpp"

using namespace v8;
using namespace node;

namespace node_ogr {

class SpatialReference: public node::ObjectWrap {
  public:
    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    static Handle<Value> New(OGRSpatialReference *srs);
    static Handle<Value> New(OGRSpatialReference *srs, bool owned);
    static Handle<Value> toString(const Arguments &args);
    static Handle<Value> clone(const Arguments &args);
    static Handle<Value> cloneGeogCS(const Arguments &args);
    static Handle<Value> exportToWKT(const Arguments &args);
    static Handle<Value> exportToPrettyWKT(const Arguments &args);
    static Handle<Value> exportToProj4(const Arguments &args);
    static Handle<Value> exportToXML(const Arguments &args);
    static Handle<Value> importFromWKT(const Arguments &args);
    static Handle<Value> importFromProj4(const Arguments &args);
    static Handle<Value> importFromEPSG(const Arguments &args);
    static Handle<Value> importFromEPSGA(const Arguments &args);
    static Handle<Value> importFromWMSAUTO(const Arguments &args);
    static Handle<Value> importFromXML(const Arguments &args);
    static Handle<Value> importFromURN(const Arguments &args);
    static Handle<Value> importFromCRSURL(const Arguments &args);
    static Handle<Value> importFromURL(const Arguments &args);
    static Handle<Value> importFromMICoordSys(const Arguments &args);
    static Handle<Value> setWellKnownGeogCS(const Arguments &args);
    static Handle<Value> setFromUserInput(const Arguments &args);
    static Handle<Value> morphToESRI(const Arguments &args);
    static Handle<Value> morphFromESRI(const Arguments &args);
    static Handle<Value> EPSGTreatsAsLatLong(const Arguments &args);
    static Handle<Value> EPSGTreatsAsNorthingEasting(const Arguments &args);
    static Handle<Value> getLinearUnits(const Arguments &args);
    static Handle<Value> getAngularUnits(const Arguments &args);
    static Handle<Value> isGeocentric(const Arguments &args);
    static Handle<Value> isProjected(const Arguments &args);
    static Handle<Value> isLocal(const Arguments &args);
    static Handle<Value> isVertical(const Arguments &args);
    static Handle<Value> isCompound(const Arguments &args);
    static Handle<Value> isSameGeogCS(const Arguments &args);
    static Handle<Value> isSameVertCS(const Arguments &args);
    static Handle<Value> isSame(const Arguments &args);
    static Handle<Value> autoIdentifyEPSG(const Arguments &args);
    static Handle<Value> getAuthorityCode(const Arguments &args);
    static Handle<Value> getAuthorityName(const Arguments &args);
    static Handle<Value> getAttrValue(const Arguments &args);

    static ObjectCache<OGRSpatialReference*> cache;

    SpatialReference();
    SpatialReference(OGRSpatialReference *srs);
    inline OGRSpatialReference *get() { return this_; }
    void dispose();

  private:
    ~SpatialReference();
    OGRSpatialReference *this_;
    bool owned_;
};

}
#define NODE_WRAPPED_SRS_EXPORT_METHOD 
#endif
