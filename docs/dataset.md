## Dataset

Note: all methods throw errors if the dataset has already been explicitly destroyed

#### Methods

- `toString()` : string
- `close()` : void
- `getGCPCount()` : int
- `getGCPProjection()` : string
- `getGCPs()` : object[]
- `setGCPs()` : void *(throws)*
- `getFileList()` : string[]
- `flush()` : void
- `getMetadata(string domain = null)` : object

#### Properties

- `description` : string
- `size` : object
- `srs` : [SpatialReference](spatialreference.md) *(settable)*
- `driver` : [Driver](driver.md)
- `geoTransform` : Number[] *(settable)*
- `bands` : DatasetBands
- `layers` : DatasetLayers

#### Bands

- `bands.get(int band_id)` : [RasterBand](rasterband.md) *(throws, 1 based)*
- `bands.count()` : int
- `bands.create(string gdal_data_type, string[] options)` : [RasterBand](rasterband.md) *(throws)*
- `bands.forEach(function iterator)` : void

#### Layers

- `layers.get(int layer_id)` : [Layer](layer.md) *(throws)*
- `layers.get(string name)` : [Layer](layer.md) *(throws)*
- `layers.count()` : int
- `layers.create(string name, SpatialReference srs = null, OGRwkbGeometryType type = wkbUnknown, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.create(string name, SpatialReference srs = null, Function geometry_constructor, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.copy (string src_lyr_name, string dst_lyr_name, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.remove (int layer_id)` : void *(throws)*
- `layers.forEach(function iterator)` : void
