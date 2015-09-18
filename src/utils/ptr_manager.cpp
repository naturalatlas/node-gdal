#include "ptr_manager.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_rasterband.hpp"
#include "../gdal_layer.hpp"

#include <sstream>

namespace node_gdal {

PtrManager::PtrManager()
	: uid(1), layers(), bands(), datasets()
{
}

PtrManager::~PtrManager()
{
}

bool PtrManager::isAlive(long uid)
{
	if(uid == 0) return true;
	return bands.count(uid) > 0 || layers.count(uid) > 0 || datasets.count(uid) > 0; 
}

long PtrManager::add(OGRLayer* ptr, long parent_uid, bool is_result_set)
{
	PtrManagerLayerItem *item = new PtrManagerLayerItem();
	item->uid = uid++;
	item->parent = datasets[parent_uid];
	item->ptr = ptr;
	item->is_result_set = is_result_set;
	layers[item->uid] = item;

	PtrManagerDatasetItem *parent = datasets[parent_uid];
	parent->layers.push_back(item);
	return item->uid;
}

long PtrManager::add(GDALRasterBand* ptr, long parent_uid)
{
	PtrManagerRasterBandItem *item = new PtrManagerRasterBandItem();
	item->uid = uid++;
	item->parent = datasets[parent_uid];
	item->ptr = ptr;
	bands[item->uid] = item;

	PtrManagerDatasetItem *parent = datasets[parent_uid];
	parent->bands.push_back(item);
	return item->uid;
}

long PtrManager::add(GDALDataset* ptr)
{
	PtrManagerDatasetItem *item = new PtrManagerDatasetItem();
	item->uid = uid++;
	item->ptr = ptr;
	datasets[item->uid] = item;
	return item->uid;
}

#if GDAL_VERSION_MAJOR < 2
long PtrManager::add(OGRDataSource* ptr)
{
	PtrManagerDatasetItem *item = new PtrManagerDatasetItem();
	item->uid = uid++;
	item->ptr_datasource = ptr;
	datasets[item->uid] = item;
	return item->uid;
}
#endif

void PtrManager::dispose(long uid)
{
	if(datasets.count(uid)) dispose(datasets[uid]);
	else if(layers.count(uid)) dispose(layers[uid]);
	else if(bands.count(uid)) dispose(bands[uid]);
}

void PtrManager::dispose(PtrManagerDatasetItem* item)
{
	datasets.erase(item->uid);

	while(!item->layers.empty()){
   		dispose(item->layers.back());
	}
	while(!item->bands.empty()){
		dispose(item->bands.back());
	}

	#if GDAL_VERSION_MAJOR < 2
	if(item->ptr_datasource) {
		Dataset::datasource_cache.erase(item->ptr_datasource);
		OGRDataSource::DestroyDataSource(item->ptr_datasource);
	}
	#endif
	if(item->ptr){
		Dataset::dataset_cache.erase(item->ptr);
		GDALClose(item->ptr);
	}

	delete item;
}

void PtrManager::dispose(PtrManagerRasterBandItem* item)
{
	RasterBand::cache.erase(item->ptr);
	bands.erase(item->uid);
	item->parent->bands.remove(item);
	delete item;
}

void PtrManager::dispose(PtrManagerLayerItem* item)
{
	Layer::cache.erase(item->ptr);
	layers.erase(item->uid);
	item->parent->layers.remove(item);

	#if GDAL_VERSION_MAJOR < 2
	OGRDataSource *parent_ds = item->parent->ptr_datasource;
	#else
	GDALDataset *parent_ds = item->parent->ptr;
	#endif

	if (item->is_result_set) {
		parent_ds->ReleaseResultSet(item->ptr);
	}

	delete item;
}

}