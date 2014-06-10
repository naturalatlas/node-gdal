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

class RasterBand: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(GDALRasterBand *band);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> getBand(const Arguments &args);
	//static Handle<Value> getDataset(const Arguments &args);
	static Handle<Value> getRasterDataType(const Arguments &args);
	static Handle<Value> getBlockSize(const Arguments &args);
	static Handle<Value> getAccess(const Arguments &args);
	//static Handle<Value> rasterIO(const Arguments &args);
	static Handle<Value> flushCache(const Arguments &args);
	static Handle<Value> getCategoryNames(const Arguments &args);
	static Handle<Value> getNoDataValue(const Arguments &args);
	static Handle<Value> getMinimum(const Arguments &args);
	static Handle<Value> getMaximum(const Arguments &args);
	static Handle<Value> getOffset(const Arguments &args);
	static Handle<Value> getScale(const Arguments &args);
	static Handle<Value> getUnitType(const Arguments &args);
	//static Handle<Value> getColorTable(const Arguments &args);
	static Handle<Value> fill(const Arguments &args);
	static Handle<Value> setCategoryNames(const Arguments &args);
	static Handle<Value> setNoDataValue(const Arguments &args);
	//static Handle<Value> setColorTable(const Arguments &args);
	static Handle<Value> setOffset(const Arguments &args);
	static Handle<Value> setScale(const Arguments &args);
	static Handle<Value> setUnitType(const Arguments &args);
	static Handle<Value> getStatistics(const Arguments &args);
	static Handle<Value> computeStatistics(const Arguments &args);
	static Handle<Value> setStatistics(const Arguments &args);
	static Handle<Value> hasArbitraryOverviews(const Arguments &args);
	static Handle<Value> getOverviewCount(const Arguments &args);
	static Handle<Value> getOverview(const Arguments &args);
	static Handle<Value> getRasterSampleOverview(const Arguments &args);
	//static Handle<Value> buildOverviews(const Arguments &args);
	//static Handle<Value> getHistogram(const Arguments &args);
	//static Handle<Value> getDefaultHistogram(const Arguments &args);
	//static Handle<Value> setDefaultHistogram(const Arguments &args);
	static Handle<Value> getMaskBand(const Arguments &args);
	static Handle<Value> getMaskFlags(const Arguments &args);
	static Handle<Value> createMaskBand(const Arguments &args);
	static Handle<Value> getMetadata(const Arguments &args);

	static Handle<Value> dsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> sizeGetter(Local<String> property, const AccessorInfo &info);

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

#endif
