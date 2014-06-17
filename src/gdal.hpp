#ifndef __NODE_GDAL_GLOBAL_H__
#define __NODE_GDAL_GLOBAL_H__

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

namespace node_gdal {
	static Handle<Value> close(const Arguments &args)
	{
		Dataset* ds;

		NODE_ARG_WRAPPED(0, "dataset", Dataset, ds);

		ds->dispose();

		return Undefined();
	}

	static Handle<Value> open(const Arguments &args)
	{
		HandleScope scope;

		std::string path;
		std::string mode = "r";
		GDALAccess access = GA_ReadOnly;

		NODE_ARG_STR(0, "path", path);
		NODE_ARG_OPT_STR(1, "mode", mode);

		if (mode == "r+") {
			access = GA_Update;
		}

		GDALDataset *ds = NULL;

		ds = (GDALDataset*) GDALOpen(path.c_str(), access);

		if (ds == NULL) {
			return NODE_THROW("Error opening dataset");
		}

		return scope.Close(Dataset::New(ds));
	}

	static Handle<Value> openShared(const Arguments &args)
	{
		HandleScope scope;

		std::string ds_name;
		GDALAccess access = GA_ReadOnly;

		NODE_ARG_STR(0, "dataset", ds_name);

		if (args.Length() > 1) {
			if (args[1]->IsInt32() || args[1]->IsBoolean()) {
				access = static_cast<GDALAccess>(args[1]->IntegerValue());
			} else {
				return NODE_THROW("Update argument must be integer or boolean");
			}
		}

		GDALDataset *ds = NULL;

		ds = (GDALDataset*) GDALOpenShared(ds_name.c_str(), access);

		if (ds == NULL) {
			return NODE_THROW("Error opening dataset");
		}

		return scope.Close(Dataset::New(ds));
	}

	static Handle<Value> getDriverByName(const Arguments &args)
	{
		HandleScope scope;

		std::string driver_name;

		NODE_ARG_STR(0, "driver name", driver_name);

		GDALDriver *driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());

		if (driver == NULL) {
			return NODE_THROW("Error retrieving driver");
		}

		return scope.Close(Driver::New(driver));
	}

	static Handle<Value> getDriverCount(const Arguments &args)
	{
		HandleScope scope;

		return scope.Close(Integer::New(GetGDALDriverManager()->GetDriverCount()));
	}

	static Handle<Value> getDriver(const Arguments &args)
	{
		HandleScope scope;

		int driver_index;

		NODE_ARG_INT(0, "driver index", driver_index);

		GDALDriver *driver = GetGDALDriverManager()->GetDriver(driver_index);

		if (driver == NULL) {
			return NODE_THROW("Error retrieving driver");
		}

		return scope.Close(Driver::New(driver));
	}

	static Handle<Value> setConfigOption(const Arguments &args)
	{
		HandleScope scope;

		std::string name;
		std::string val;
		NODE_ARG_STR(0, "name", name);
		NODE_ARG_STR(1, "value", val);

		CPLSetConfigOption(name.c_str(), val.c_str());

		return Undefined();
	}

	static Handle<Value> getConfigOption(const Arguments &args)
	{
		HandleScope scope;

		std::string name;
		NODE_ARG_STR(0, "name", name);

		return scope.Close(SafeString::New(CPLGetConfigOption(name.c_str(), NULL)));
	}
}

#endif
