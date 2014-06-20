#ifndef __NODE_GDAL_RASTERBAND_H__
#define __NODE_GDAL_RASTERBAND_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

#include "obj_cache.hpp"

using namespace v8;
using namespace node;

namespace node_gdal {

class RasterBand: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALRasterBand *band);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> flush(const Arguments &args);
	static Handle<Value> fill(const Arguments &args);
	static Handle<Value> getStatistics(const Arguments &args);
	static Handle<Value> computeStatistics(const Arguments &args);
	static Handle<Value> setStatistics(const Arguments &args);
	static Handle<Value> getMaskBand(const Arguments &args);
	static Handle<Value> getMaskFlags(const Arguments &args);
	static Handle<Value> createMaskBand(const Arguments &args);

	// unimplemented methods
	//static Handle<Value> getColorTable(const Arguments &args);
	//static Handle<Value> setColorTable(const Arguments &args);
	//static Handle<Value> rasterIO(const Arguments &args);
	//static Handle<Value> buildOverviews(const Arguments &args);
	//static Handle<Value> getHistogram(const Arguments &args);
	//static Handle<Value> getDefaultHistogram(const Arguments &args);
	//static Handle<Value> setDefaultHistogram(const Arguments &args);

	static Handle<Value> dsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> sizeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> idGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> overviewsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> blockSizeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> minimumGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> maximumGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> readOnlyGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> dataTypeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> hasArbitraryOverviewsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> unitTypeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> scaleGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> offsetGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> noDataValueGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> categoryNamesGetter(Local<String> property, const AccessorInfo &info);

	static void unitTypeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void scaleSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void offsetSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void noDataValueSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void categoryNamesSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);

	static ObjectCache<GDALRasterBand*> cache;

	RasterBand();
	RasterBand(GDALRasterBand *band);
	inline GDALRasterBand *get() {
		return this_;
	}
	void dispose();

private:
	~RasterBand();
	GDALRasterBand *this_;
};

}
#endif
