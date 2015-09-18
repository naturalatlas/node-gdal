
#ifndef __PTR_MANAGER_H__
#define __PTR_MANAGER_H__

// node
#include <node.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// gdal
#include <gdal_priv.h>

// ogr
#include <ogrsf_frmts.h>

#include <map>
#include <list>

using namespace v8;

struct PtrManagerDatasetItem;
struct PtrManagerLayerItem {
	long uid;
	PtrManagerDatasetItem *parent;
	OGRLayer *ptr;
	bool is_result_set;
};

struct PtrManagerRasterBandItem {
	long uid;
	PtrManagerDatasetItem *parent;
	GDALRasterBand *ptr;
};

struct PtrManagerDatasetItem {
	long uid;
	std::list<PtrManagerLayerItem*> layers;
	std::list<PtrManagerRasterBandItem*> bands;
	GDALDataset *ptr;
	#if GDAL_VERSION_MAJOR < 2
	OGRDataSource *ptr_datasource;
	#endif
};

namespace node_gdal {

// A class for cleaning up GDAL objects that depend on open datasets 

class PtrManager {
public:
	long add(GDALDataset* ptr);
	#if GDAL_VERSION_MAJOR < 2
	long add(OGRDataSource* ptr);
	#endif
	long add(GDALRasterBand* ptr, long parent_uid);
	long add(OGRLayer* ptr, long parent_uid, bool is_result_set);
	void dispose(long uid);
	bool isAlive(long uid);

	PtrManager();
	~PtrManager();
private:
	long uid;
	void dispose(PtrManagerLayerItem* item);
	void dispose(PtrManagerRasterBandItem* item);
	void dispose(PtrManagerDatasetItem* item);
	std::map<long, PtrManagerLayerItem*> layers;
	std::map<long, PtrManagerRasterBandItem*> bands;
	std::map<long, PtrManagerDatasetItem*> datasets;
};

}

#endif