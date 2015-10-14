
#ifndef __WARP_OPTIONS_H__
#define __WARP_OPTIONS_H__

// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal_priv.h>
#include <gdalwarper.h>

#include "string_list.hpp"
#include "number_list.hpp"

using namespace v8;

namespace node_gdal {

// A class for parsing a V8::Value and constructing a GDALWarpOptions struct
//
// see: http://www.gdal.org/structGDALWarpOptions.html
//
// {
//   options : string[] | object
//   memoryLimit : int
//   resampleAlg : string
//   src: Dataset
//   dst: Dataset
//   srcBands: int | int[]
//   dstBands: int | int[]
//   nBands: int
//   srcAlphaBand: int
//   dstAlphaBand: int
//   srcNoData: double
//   dstNoData: double
//   cutline: geometry
//   blend: double
// }

class WarpOptions {
public:
	int parse(Local<Value> value);
	int parseResamplingAlg(Local<Value> value);

	WarpOptions();
	~WarpOptions();

	inline GDALWarpOptions* get() {
		return options;
	}
	inline bool useMultithreading() {
		return multi;
	}
private:
	GDALWarpOptions *options;
	StringList additional_options;
	IntegerList src_bands;
	IntegerList dst_bands;
	double* src_nodata;
	double* dst_nodata;
	bool multi;
};

}

#endif