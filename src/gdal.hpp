#ifndef __NODE_OGR_GLOBAL_H__
#define __NODE_OGR_GLOBAL_H__

// v8
#include <v8.h>

// node
#include <node.h>

// gdal
#include "gdal_common.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"

using namespace v8;
using namespace node;

namespace node_ogr {
	static Handle<Value> open(const Arguments &args) {
		HandleScope scope;

		std::string ds_name;
		GDALAccess access = GA_ReadOnly;

		NODE_ARG_STR(0, "dataset", ds_name);
		NODE_ARG_ENUM_OPT(1, "update", GDALAccess, access);

		GDALDataset *ds = NULL;

		ds = (GDALDataset*) GDALOpen(ds_name.c_str(), access);

		if (ds == NULL) {
			return ThrowException(Exception::Error(String::New("Error opening dataset")));
		}

		return scope.Close(Dataset::New(ds));
	}

	static Handle<Value> openShared(const Arguments &args) {
		HandleScope scope;

		std::string ds_name;
		GDALAccess access = GA_ReadOnly;

		NODE_ARG_STR(0, "dataset", ds_name);
		NODE_ARG_ENUM_OPT(1, "update", GDALAccess, access);

		GDALDataset *ds = NULL;

		ds = (GDALDataset*) GDALOpenShared(ds_name.c_str(), access);

		if (ds == NULL) {
			return ThrowException(Exception::Error(String::New("Error opening dataset")));
		}

		return scope.Close(Dataset::New(ds));
	}

	static Handle<Value> getDriverByName(const Arguments &args) {
		HandleScope scope;

		std::string driver_name;

		NODE_ARG_STR(0, "driver name", driver_name);

		GDALDriver *driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());

		if (driver == NULL) {
			return ThrowException(Exception::Error(String::New("Error retrieving driver")));
		}

		return scope.Close(Driver::New(driver));
	}

	static Handle<Value> getDriverCount(const Arguments &args) {
		HandleScope scope;

		return scope.Close(Integer::New(GetGDALDriverManager()->GetDriverCount()));
	}

	static Handle<Value> getDriver(const Arguments &args) {
		HandleScope scope;

		int driver_index;

		NODE_ARG_INT(0, "driver index", driver_index);

		return scope.Close(Driver::New(GetGDALDriverManager()->GetDriver(driver_index)));
	}

}

#endif
