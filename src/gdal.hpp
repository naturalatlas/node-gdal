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
		NanScope();

		std::string path;
		std::string mode = "r";

		NODE_ARG_STR(0, "path", path);
		NODE_ARG_OPT_STR(1, "mode", mode);

		#if GDAL_VERSION_MAJOR < 2
			GDALAccess access = GA_ReadOnly;
			if (mode == "r+") {
				access = GA_Update;
			} else if (mode != "r") {
				NanThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
				NanReturnUndefined();
			}

			OGRDataSource *ogr_ds = OGRSFDriverRegistrar::Open(path.c_str(), static_cast<int>(access));
			if(ogr_ds) {
				NanReturnValue(Dataset::New(ogr_ds));
			}

			GDALDataset *gdal_ds = (GDALDataset*) GDALOpen(path.c_str(), access);
			if(gdal_ds) {
				NanReturnValue(Dataset::New(gdal_ds));
			}
		#else
			unsigned int flags = 0;
			if (mode == "r+") {
				flags |= GDAL_OF_UPDATE;
			} else if (mode == "r") {
				flags |= GDAL_OF_READONLY;
			} else {
				NanThrowError("Invalid open mode. Must be \"r\" or \"r+\"");
				NanReturnUndefined();
			}

			GDALDataset *ds = (GDALDataset*) GDALOpenEx(path.c_str(), flags, NULL, NULL, NULL);
			if(ds) {
				NanReturnValue(Dataset::New(ds));
			}
		#endif

		NanThrowError("Error opening dataset");
		NanReturnUndefined();
	}

	static NAN_METHOD(setConfigOption)
	{
		NanScope();

		std::string name;

		NODE_ARG_STR(0, "name", name);

		if (args.Length() < 2) {
			NanThrowError("string or null value must be provided");
			NanReturnUndefined();
		}
		if(args[1]->IsString()){
			std::string val = *NanUtf8String(args[1]);
			CPLSetConfigOption(name.c_str(), val.c_str());
		} else if(args[1]->IsNull() || args[1]->IsUndefined()) {
			CPLSetConfigOption(name.c_str(), NULL);
		} else {
			NanThrowError("value must be a string or null");
			NanReturnUndefined();
		}

		NanReturnUndefined();
	}

	static NAN_METHOD(getConfigOption)
	{
		NanScope();

		std::string name;
		NODE_ARG_STR(0, "name", name);

		NanReturnValue(SafeString::New(CPLGetConfigOption(name.c_str(), NULL)));
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
		NanScope();

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
			NanThrowError("Axis must be 'lat' or 'long'");
			NanReturnUndefined();
		}

		NanReturnValue(SafeString::New(GDALDecToDMS(angle, axis.c_str(), precision)));
	}
}

#endif
