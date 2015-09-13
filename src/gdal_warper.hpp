#ifndef __GDAL_WARPER_H__
#define __GDAL_WARPER_H__

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

// Methods and classes from gdalwarper.h
// http://www.gdal.org/gdalwarper_8h.html

namespace node_gdal {
namespace Warper {

	void Initialize(Local<Object> target);

	NAN_METHOD(reprojectImage);
	NAN_METHOD(suggestedWarpOutput);

}
}

#endif