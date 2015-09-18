#ifndef __NODE_GDAL_GLOBAL_H__
#define __NODE_GDAL_GLOBAL_H__

// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// ogr
#include <ogr_api.h>
#include <ogrsf_frmts.h>

// gdal
#include "gdal_common.hpp"
#include "gdal_driver.hpp"
#include "gdal_dataset.hpp"

using namespace v8;
using namespace node;

namespace node_gdal {

	static NAN_METHOD(open)
	{
		Nan::HandleScope scope;

		std::string path;
		std::string mode = "r";

		NODE_ARG_STR(0, "path", path);
		NODE_ARG_OPT_STR(1, "mode", mode);

		#if GDAL_VERSION_MAJOR < 2
			GDALAccess access = GA_ReadOnly;
			if (mode == "r+") {
				access = GA_Update;
			} else if (mode != "r") {
				Nan::ThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
				return;
			}

			OGRDataSource *ogr_ds = OGRSFDriverRegistrar::Open(path.c_str(), static_cast<int>(access));
			if(ogr_ds) {
				info.GetReturnValue().Set(Dataset::New(ogr_ds));
				return;
			}

			GDALDataset *gdal_ds = (GDALDataset*) GDALOpen(path.c_str(), access);
			if(gdal_ds) {
				info.GetReturnValue().Set(Dataset::New(gdal_ds));
				return;
			}
		#else
			unsigned int flags = 0;
			if (mode == "r+") {
				flags |= GDAL_OF_UPDATE;
			} else if (mode == "r") {
				flags |= GDAL_OF_READONLY;
			} else {
				Nan::ThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
				return;
			}

			GDALDataset *ds = (GDALDataset*) GDALOpenEx(path.c_str(), flags, NULL, NULL, NULL);
			if(ds) {
				info.GetReturnValue().Set(Dataset::New(ds));
				return;
			}
		#endif

		Nan::ThrowError("Error opening dataset");
		return;
	}

	static NAN_METHOD(setConfigOption)
	{
		Nan::HandleScope scope;

		std::string name;

		NODE_ARG_STR(0, "name", name);

		if (info.Length() < 2) {
			Nan::ThrowError("string or null value must be provided");
			return;
		}
		if(info[1]->IsString()){
			std::string val = *Nan::Utf8String(info[1]);
			CPLSetConfigOption(name.c_str(), val.c_str());
		} else if(info[1]->IsNull() || info[1]->IsUndefined()) {
			CPLSetConfigOption(name.c_str(), NULL);
		} else {
			Nan::ThrowError("value must be a string or null");
			return;
		}

		return;
	}

	static NAN_METHOD(getConfigOption)
	{
		Nan::HandleScope scope;

		std::string name;
		NODE_ARG_STR(0, "name", name);

		info.GetReturnValue().Set(SafeString::New(CPLGetConfigOption(name.c_str(), NULL)));
	}

	/**
	 * Convert decimal degrees to degrees, minutes, and seconds string
	 *
	 * @for gdal
	 * @static
	 * @method decToDMS
	 * @param {Number} angle
	 * @param {String} axis `"lat"` or `"long"`
	 * @param {Integer} [precision=2]
	 * @return {String} A string nndnn'nn.nn'"L where n is a number and L is either N or E
	 */
	static NAN_METHOD(decToDMS){
		Nan::HandleScope scope;

		double angle;
		std::string axis;
		int precision = 2;
		NODE_ARG_DOUBLE(0, "angle", angle);
		NODE_ARG_STR(1, "axis", axis);
		NODE_ARG_INT_OPT(2, "precision", precision);

		if (axis.length() > 0) {
			axis[0] = toupper(axis[0]);
		}
		if (axis != "Lat" && axis != "Long") {
			Nan::ThrowError("Axis must be 'lat' or 'long'");
			return;
		}

		info.GetReturnValue().Set(SafeString::New(GDALDecToDMS(angle, axis.c_str(), precision)));
	}
}

#endif
