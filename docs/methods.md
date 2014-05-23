## Global Methods

- `open(string ds_name, bool update = false) : Dataset #throws` //todo: test giving integer and boolean to update arg
- `openShared(string ds_name, bool update = false) : Dataset #throws`
- `close(Dataset ds) : void`
- `getDriver() : Driver #throws`
- `getDriverByName() : Driver #throws`
- `getDriverCount() : integer`

## Dataset

- `toString() : string`
- `getMetadata(string domain = null) : object`
- `close() : void`
- `getRasterXSize() : integer`
- `getRasterYSize() : integer`
- `getRasterCount() : integer`
- `getRasterBand(integer band_id) : RasterBand`
- `getProjectionRef() : string`
- `setProjection(string wkt_or_proj4) : void #throws`
- `addBand(integer gdal_data_type, string[] options) : RasterBand #throws`
- `getDriver() : Driver`
- `getGCPCount() : integer`
- `getGCPProjection() : string`
- `createMaskBand(integer flags) : RasterBand`
- `getGeoTransform() : Number[]`
- `setGeoTransform(Number[] transform) #throws`
- `getGCPs() : object[]`
- `setGCPs() : void #throws`
- `getFileList() : string[]`
- `flushCache() : void`

## Driver

- `toString() : string`
- `getMetadata(string domain = null) : object`
- `create(string filename, integer x_size, integer y_size, int n_bands = 1, int gdal_data_type = GDT_Byte, string[] co): Dataset #throws`
- `createCopy(string filename, Dataset src, boolean strict = false, string[] options = null): Dataset #throws`
- `deleteDataset() : void #throws`
- `quietDelete() : void #throws`
- `rename(string new_name, string old_name): void #throws`
- `copyFiles(string new_name, string old_name) : void #throws`

### Properties

- `LongName : string`
- `ShortName : string`

## RasterBand

- `toString()`
- `getMetadata(string domain = null) : object`
- `getXSize()`
- `getYSize()`
- `getBand()`
- `getDataset()`
- `getRasterDataType()`
- `getBlockSize()`
- `getAccess()`
- `flushCache()`
- `getCategoryNames()`
- `getNoDataValue()`
- `getMinimum()`
- `getMaximum()`
- `getOffset()`
- `getScale()`
- `getUnitType()`
- `fill()`
- `setCategoryNames()`
- `setNoDataValue()`
- `setOffset()`
- `setScale()`
- `setUnitType()`
- `getStatistics()`
- `computeStatistics()`
- `setStatistics()`
- `hasArbitraryOverviews()`
- `getOverviewCount()`
- `getOverview()`
- `getRasterSampleOverview()`
- `getMaskBand()`
- `getMaskFlags()`
- `createMaskBand()`