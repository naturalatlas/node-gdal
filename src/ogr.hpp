#ifndef __NODE_OGR_GLOBAL_H__
#define __NODE_OGR_GLOBAL_H__

// v8
#include <v8.h>

// node
#include <node.h>

// ogr
#include "ogr_common.hpp"
#include "ogr_driver.hpp"
#include "ogr_datasource.hpp"
#include "ogr_spatial_reference.hpp"
#include "ogr_geometry.hpp"

namespace node_ogr {

  using namespace v8;
  using namespace node;

  void Init(Handle<Object> target);

  Handle<Value> open(const Arguments &args);
  Handle<Value> getDriverByName(const Arguments &args);
  Handle<Value> getDriverCount(const Arguments &args);
  Handle<Value> getDriver(const Arguments &args);
  Handle<Value> getOpenDSCount(const Arguments &args);
  Handle<Value> getOpenDS(const Arguments &args);
  Handle<Value> createGeometryFromWkt(const Arguments &args);

}

#endif
