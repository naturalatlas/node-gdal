## Global Methods

- `open(string ds_name, bool update = false) : Dataset #throws` //todo: test giving integer and boolean to update arg
- `openShared(string ds_name, bool update = false) : Dataset #throws`
- `close(Dataset ds) : void`
- `getDriver() : Driver #throws`
- `getDriverByName() : Driver #throws`
- `getDriverCount() : integer`

## Dataset

Note: all methods throw errors if the dataset has already been explicitly destroyed

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

### Proposed Properties

- `size : object`
- `x_size : integer //width?`
- `y_size : integer //height?`
- `raster_count : integer`
- `raster_bands : RasterBand[]`
- `driver : Driver`

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

Note: all methods throw errors if the band has been destroyed by the dataset

- `toString() : string`
- `getMetadata(string domain = null) : object`
- `getXSize() : integer`
- `getYSize() : integer`
- `getBand(integer i) : RasterBand`
- `getDataset() : Dataset`
- `getRasterDataType() : integer`
- `getBlockSize() : object`
- `getAccess() : integer`
- `flushCache() : void`
- `getCategoryNames() : string[]`
- `getNoDataValue() : Number`
- `getMinimum() : Number`
- `getMaximum() : Number`
- `getOffset() : Number`
- `getScale() : Number`
- `getUnitType() : string`
- `fill(number real_value, number imag_value = 0) : void #throws`
- `setCategoryNames(string[] names) : void #throws`
- `setNoDataValue(Number no_data_val) : void #throws`
- `setOffset(Number offset) : void #throws`
- `setScale(Number scale) : void #throws`
- `setUnitType(string type) : void #throws`
- `getStatistics(boolean allow_approx, boolean force) : object #throws`
- `computeStatistics(boolean allow_approx) : object #throws`
- `setStatistics(Number min, Number max, Number mean, Number std_dev) : void #throws`
- `hasArbitraryOverviews() : boolean`
- `getOverviewCount() : integer`
- `getOverview(integer i) : RasterBand`
- `getRasterSampleOverview(integer num_samples) : RasterBand`
- `getMaskBand() : RasterBand`
- `getMaskFlags() : integer`
- `createMaskBand(integer num_samples) : void #throws`

### Proposed Properties

- `size : object`
- `x_size : integer //width?`
- `y_size : integer //height?`
- `block_size : object`
- `unit_type : string`
- `scale : Number`
- `offset : Number`
- `minimum : Number`
- `maximum : Number`