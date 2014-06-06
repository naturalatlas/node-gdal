// v8
#include <v8.h>

// node
#include <node.h>
#include <node_buffer.h>
#include <node_version.h>

// ogr
#include <ogr_api.h>

// node-ogr
#include "ogr_common.hpp"
#include "ogr.hpp"
#include "ogr_driver.hpp"
#include "ogr_datasource.hpp"
#include "ogr_layer.hpp"
#include "ogr_feature_defn.hpp"
#include "ogr_field_defn.hpp"
#include "ogr_feature.hpp"
#include "ogr_geometry.hpp"
#include "ogr_spatial_reference.hpp"
#include "ogr_coordinate_transformation.hpp"

// std
#include <string>
#include <sstream>
#include <vector>

using namespace node;
using namespace v8;

namespace node_ogr {

  /*enum {
    Success = 0,
    NotEnoughData = 1,
    NotEnoughMemory = 2,
    UnsupportedGeometryType = 3,
    UnsupportedOperation = 4,
    CorruptData = 5,
    Failure = 6,
    UnsupportedSRS = 7,
    InvalidHandle = 8
  } Error;*/

  void Init(Handle<Object> target)
  {
    NODE_SET_METHOD(target, "open", node_ogr::open);
    NODE_SET_METHOD(target, "getDriverByName", node_ogr::getDriverByName);
    NODE_SET_METHOD(target, "getDriverCount", node_ogr::getDriverCount);
    NODE_SET_METHOD(target, "getDriver", node_ogr::getDriver);
    NODE_SET_METHOD(target, "getOpenDSCount", node_ogr::getOpenDSCount);
    NODE_SET_METHOD(target, "getOpenDS", node_ogr::getOpenDS);
    NODE_SET_METHOD(target, "createGeometryFromWkt", node_ogr::createGeometryFromWkt);

    Driver::Initialize(target);
    Datasource::Initialize(target);
    Layer::Initialize(target);
    Feature::Initialize(target);
    FeatureDefn::Initialize(target);
    FieldDefn::Initialize(target);
    Geometry::Initialize(target);
    SpatialReference::Initialize(target);
    CoordinateTransformation::Initialize(target);

    OGRRegisterAll();
   
    OGRSFDriverRegistrar *reg = OGRSFDriverRegistrar::GetRegistrar();

    int driver_count = reg->GetDriverCount();

    Local<Array> supported_drivers = Array::New(driver_count);

    for (int i = 0; i < driver_count; ++i) {
      OGRSFDriver *driver = reg->GetDriver(i);
      supported_drivers->Set(Integer::New(static_cast<int>(i)), String::New(driver->GetName()));
    }

    target->Set(String::NewSymbol("drivers"), supported_drivers);

    NODE_DEFINE_CONSTANT(target, wkbUnknown);
    NODE_DEFINE_CONSTANT(target, wkbPoint);
    NODE_DEFINE_CONSTANT(target, wkbLineString);
    NODE_DEFINE_CONSTANT(target, wkbPolygon);
    NODE_DEFINE_CONSTANT(target, wkbMultiPoint);
    NODE_DEFINE_CONSTANT(target, wkbMultiLineString);
    NODE_DEFINE_CONSTANT(target, wkbMultiPolygon);
    NODE_DEFINE_CONSTANT(target, wkbGeometryCollection);
    NODE_DEFINE_CONSTANT(target, wkbNone);
    NODE_DEFINE_CONSTANT(target, wkbLinearRing);
    NODE_DEFINE_CONSTANT(target, wkbPoint25D);
    NODE_DEFINE_CONSTANT(target, wkbLineString25D);
    NODE_DEFINE_CONSTANT(target, wkbPolygon25D);
    NODE_DEFINE_CONSTANT(target, wkbMultiPoint25D);
    NODE_DEFINE_CONSTANT(target, wkbMultiLineString25D);
    NODE_DEFINE_CONSTANT(target, wkbMultiPolygon25D);
    NODE_DEFINE_CONSTANT(target, wkbGeometryCollection25D);

    NODE_DEFINE_CONSTANT(target, OFTInteger);
    NODE_DEFINE_CONSTANT(target, OFTIntegerList);
    NODE_DEFINE_CONSTANT(target, OFTReal);
    NODE_DEFINE_CONSTANT(target, OFTRealList);
    NODE_DEFINE_CONSTANT(target, OFTString);
    NODE_DEFINE_CONSTANT(target, OFTStringList);
    NODE_DEFINE_CONSTANT(target, OFTWideString);
    NODE_DEFINE_CONSTANT(target, OFTWideStringList);
    NODE_DEFINE_CONSTANT(target, OFTBinary);
    NODE_DEFINE_CONSTANT(target, OFTDate);
    NODE_DEFINE_CONSTANT(target, OFTTime);
    NODE_DEFINE_CONSTANT(target, OFTDateTime);

    target->Set(String::NewSymbol("CreateDataSourceOption"), String::New(ODrCCreateDataSource));
    target->Set(String::NewSymbol("DeleteDataSourceOption"), String::New(ODrCDeleteDataSource));
  }

  Handle<Value> open(const Arguments &args) {
    HandleScope scope;

    std::string ds_name;
    bool is_update = false;

    NODE_ARG_STR(0, "datasource", ds_name);
    NODE_ARG_BOOL_OPT(1, "update", is_update);

    OGRDataSource *ds = NULL;

    ds = OGRSFDriverRegistrar::Open(ds_name.c_str(), is_update);

    if (ds == NULL) {
      return ThrowException(Exception::Error(String::New("Error opening datasource")));
    }

    return scope.Close(Datasource::New(ds));
  }

  Handle<Value> getDriverByName(const Arguments &args) {
    HandleScope scope;

    std::string driver_name;

    NODE_ARG_STR(0, "driver name", driver_name);

    OGRSFDriver *driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver_name.c_str());

    if (driver == NULL) {
      return ThrowException(Exception::Error(String::New("Error retrieving driver")));
    }

    return scope.Close(Driver::New(driver));
  }

  Handle<Value> getDriverCount(const Arguments &args) {
    HandleScope scope;

    return scope.Close(Integer::New(OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount()));
  }

  Handle<Value> getDriver(const Arguments &args) {
    HandleScope scope;

    int driver_index;

    NODE_ARG_INT(0, "driver index", driver_index);

    OGRSFDriver *driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(driver_index);

    if (driver == NULL) {
      return ThrowException(Exception::Error(String::New("Error retrieving driver")));
    }

    return scope.Close(Driver::New(driver));
  }

  Handle<Value> getOpenDSCount(const Arguments &args) {
    HandleScope scope;

    return scope.Close(Integer::New(OGRSFDriverRegistrar::GetRegistrar()->GetOpenDSCount()));
  }

  Handle<Value> getOpenDS(const Arguments &args) {
    HandleScope scope;

    int ds_index;

    NODE_ARG_INT(0, "data source index", ds_index);

    return scope.Close(Datasource::New(OGRSFDriverRegistrar::GetRegistrar()->GetOpenDS(ds_index)));
  }

  Handle<Value> createGeometryFromWkt(const Arguments &args) {
    HandleScope scope;

    std::string wkt_string; 
    SpatialReference *srs = NULL;

    NODE_ARG_STR(0, "wkt", wkt_string);
    NODE_ARG_WRAPPED_OPT(1, "srs", SpatialReference, srs);

    char *wkt = (char*) wkt_string.c_str(); 
    OGRGeometry *geom = NULL;
    OGRSpatialReference *ogr_srs = NULL;
    if(srs) 
      ogr_srs = srs->get();

    if(OGRGeometryFactory::createFromWkt(&wkt, ogr_srs, &geom)){
      return NODE_THROW("Error creating geometry");
    }

    return scope.Close(Geometry::New(geom, true));
  }

} // namespace node_ogr

//NODE_MODULE(ogr, node_ogr::Init)
