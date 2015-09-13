#ifndef __GDAL_ALGORITHMS_H__
#define __GDAL_ALGORITHMS_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal_priv.h>
#include <gdal_alg.h>

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

// Methods from gdal_alg.h
// http://www.gdal.org/gdal__alg_8h.html

namespace node_gdal {
namespace Algorithms {

	void Initialize(Local<Object> target);

	NAN_METHOD(fillNodata);
	NAN_METHOD(contourGenerate);
	NAN_METHOD(sieveFilter);
	NAN_METHOD(checksumImage);
	NAN_METHOD(polygonize);
}
}

#endif