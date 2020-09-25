#ifndef __NODE_OGR_GEOMETRY_H__
#define __NODE_OGR_GEOMETRY_H__

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

using namespace v8;
using namespace node;

namespace node_gdal {

class Geometry: public Nan::ObjectWrap {
	friend class Feature;

public:
	static Nan::Persistent<FunctionTemplate> constructor;

	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRGeometry *geom);
	static Local<Value> New(OGRGeometry *geom, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(isEmpty);
	static NAN_METHOD(isValid);
	static NAN_METHOD(isSimple);
	static NAN_METHOD(isRing);
	static NAN_METHOD(clone);
	static NAN_METHOD(empty);
	static NAN_METHOD(exportToKML);
	static NAN_METHOD(exportToGML);
	static NAN_METHOD(exportToJSON);
	static NAN_METHOD(exportToWKT);
	static NAN_METHOD(exportToWKB);
	static NAN_METHOD(closeRings);
	static NAN_METHOD(segmentize);
	static NAN_METHOD(intersects);
	static NAN_METHOD(equals);
	static NAN_METHOD(disjoint);
	static NAN_METHOD(touches);
	static NAN_METHOD(crosses);
	static NAN_METHOD(within);
	static NAN_METHOD(contains);
	static NAN_METHOD(overlaps);
	static NAN_METHOD(boundary);
	static NAN_METHOD(distance);
	static NAN_METHOD(convexHull);
	static NAN_METHOD(buffer);
	static NAN_METHOD(intersection);
	static NAN_METHOD(unionGeometry);
	static NAN_METHOD(difference);
	static NAN_METHOD(symDifference);
	static NAN_METHOD(centroid);
	static NAN_METHOD(simplify);
	static NAN_METHOD(simplifyPreserveTopology);
	static NAN_METHOD(polygonize);
	static NAN_METHOD(swapXY);
	static NAN_METHOD(getNumGeometries);
	static NAN_METHOD(getEnvelope);
	static NAN_METHOD(getEnvelope3D);
	static NAN_METHOD(transform);
	static NAN_METHOD(transformTo);

	//static constructor methods
	static NAN_METHOD(create);
	static NAN_METHOD(createFromWkt);
	static NAN_METHOD(createFromWkb);
	static NAN_METHOD(createFromGeoJson);
	static NAN_METHOD(getName);
	static NAN_METHOD(getConstructor);

	static NAN_GETTER(srsGetter);
	static NAN_GETTER(typeGetter);
	static NAN_GETTER(nameGetter);
	static NAN_GETTER(wkbSizeGetter);
	static NAN_GETTER(dimensionGetter);
	static NAN_GETTER(coordinateDimensionGetter);

	static NAN_SETTER(srsSetter);
	static NAN_SETTER(coordinateDimensionSetter);

	static OGRwkbGeometryType getGeometryType_fixed(OGRGeometry* geom);
	static Local<Value> getConstructor(OGRwkbGeometryType type);

	Geometry();
	Geometry(OGRGeometry *geom);
	inline OGRGeometry *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

protected:
	~Geometry();
	OGRGeometry *this_;
	bool owned_;
	int size_;
};

#define UPDATE_AMOUNT_OF_GEOMETRY_MEMORY(geom) {                                        \
    int new_size = geom->this_->WkbSize();                                              \
    if (geom->owned_) Nan::AdjustExternalMemory(new_size - geom->size_); \
    geom->size_ = new_size;                                                             \
}

}
#endif
