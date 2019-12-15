#ifndef __NODE_OGR_COORDINATETRANSFORMATION_H__
#define __NODE_OGR_COORDINATETRANSFORMATION_H__

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

// gdal
#include <gdalwarper.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class CoordinateTransformation: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRCoordinateTransformation *transform);
	static NAN_METHOD(toString);
	static NAN_METHOD(transformPoint);

	CoordinateTransformation();
	CoordinateTransformation(OGRCoordinateTransformation *srs);
	inline OGRCoordinateTransformation *get() {
		return this_;
	}
    inline bool isAlive(){
        return this_;
    }

private:
	~CoordinateTransformation();
	OGRCoordinateTransformation *this_;
};

// adapted from gdalwarp source

class GeoTransformTransformer : public OGRCoordinateTransformation {
public:
    void* hSrcImageTransformer;

    virtual OGRSpatialReference* GetSourceCS() override { return nullptr; }
    virtual OGRSpatialReference* GetTargetCS() override { return nullptr; }

		// only used on GDAL 2.X
		virtual int TransformEx( int nCount,
                             double *x, double *y, double *z = NULL,
                             int *pabSuccess = NULL ) {
        return GDALGenImgProjTransform( hSrcImageTransformer, TRUE,
                                        nCount, x, y, z, pabSuccess );
    }

		// GDAL 3.x below
    virtual int Transform(int nCount,
        double* x, double* y, double* z = NULL)
    {
        int nResult;

        int* pabSuccess = (int*)CPLCalloc(sizeof(int), nCount);
        nResult = Transform(nCount, x, y, z, pabSuccess);
        CPLFree(pabSuccess);

        return nResult;
    }

    int Transform(int nCount,
        double* x, double* y, double* z,
        int* pabSuccess)
    {
        return GDALGenImgProjTransform(hSrcImageTransformer, TRUE,
            nCount, x, y, z, pabSuccess);
    }

    virtual int Transform(int nCount,
        double* x, double* y, double* z, double* /* t */,
        int* pabSuccess)
    {
        return GDALGenImgProjTransform(hSrcImageTransformer, TRUE,
            nCount, x, y, z, pabSuccess);
    }
};
}
#endif
