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

class GeoTransformTransformer : public OGRCoordinateTransformation
{
public:

    void         *hSrcImageTransformer;

    virtual OGRSpatialReference *GetSourceCS() { return NULL; }
    virtual OGRSpatialReference *GetTargetCS() { return NULL; }

    virtual int Transform( int nCount,
                           double *x, double *y, double *z = NULL ) {
        int nResult;

        int *pabSuccess = (int *) CPLCalloc(sizeof(int),nCount);
        nResult = TransformEx( nCount, x, y, z, pabSuccess );
        CPLFree( pabSuccess );

        return nResult;
    }

    virtual int TransformEx( int nCount,
                             double *x, double *y, double *z = NULL,
                             int *pabSuccess = NULL ) {
        return GDALGenImgProjTransform( hSrcImageTransformer, TRUE,
                                        nCount, x, y, z, pabSuccess );
    }

    virtual ~GeoTransformTransformer() {
		if(hSrcImageTransformer){
			GDALDestroyGenImgProjTransformer( hSrcImageTransformer );
		}
    }
};

}
#endif
