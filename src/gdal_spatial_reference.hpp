#ifndef __NODE_OGR_SPATIALREFERENCE_H__
#define __NODE_OGR_SPATIALREFERENCE_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// ogr
#include <ogrsf_frmts.h>
#include "utils/obj_cache.hpp"

using namespace v8;
using namespace node;

namespace node_gdal {

class SpatialReference: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);

	static NAN_METHOD(New);
	static Local<Value> New(OGRSpatialReference *srs);
	static Local<Value> New(OGRSpatialReference *srs, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(clone);
	static NAN_METHOD(cloneGeogCS);
	static NAN_METHOD(exportToWKT);
	static NAN_METHOD(exportToPrettyWKT);
	static NAN_METHOD(exportToProj4);
	static NAN_METHOD(exportToXML);
	static NAN_METHOD(setWellKnownGeogCS);
	static NAN_METHOD(morphToESRI);
	static NAN_METHOD(morphFromESRI);
	static NAN_METHOD(EPSGTreatsAsLatLong);
	static NAN_METHOD(EPSGTreatsAsNorthingEasting);
	static NAN_METHOD(getLinearUnits);
	static NAN_METHOD(getAngularUnits);
	static NAN_METHOD(isGeocentric);
	static NAN_METHOD(isGeographic);
	static NAN_METHOD(isProjected);
	static NAN_METHOD(isLocal);
	static NAN_METHOD(isVertical);
	static NAN_METHOD(isCompound);
	static NAN_METHOD(isSameGeogCS);
	static NAN_METHOD(isSameVertCS);
	static NAN_METHOD(isSame);
	static NAN_METHOD(autoIdentifyEPSG);
	static NAN_METHOD(getAuthorityCode);
	static NAN_METHOD(getAuthorityName);
	static NAN_METHOD(getAttrValue);
	static NAN_METHOD(validate);

	static NAN_METHOD(fromUserInput);
	static NAN_METHOD(fromWKT);
	static NAN_METHOD(fromProj4);
	static NAN_METHOD(fromEPSG);
	static NAN_METHOD(fromEPSGA);
	static NAN_METHOD(fromESRI);
	static NAN_METHOD(fromWMSAUTO);
	static NAN_METHOD(fromXML);
	static NAN_METHOD(fromURN);
	static NAN_METHOD(fromCRSURL);
	static NAN_METHOD(fromURL);
	static NAN_METHOD(fromMICoordSys);

	static ObjectCache<OGRSpatialReference, SpatialReference> cache;

	SpatialReference();
	SpatialReference(OGRSpatialReference *srs);
	inline OGRSpatialReference *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}
	void dispose();

private:
	~SpatialReference();
	OGRSpatialReference *this_;
	bool owned_;
};

}
#endif
