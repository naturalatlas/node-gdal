## Dataset

Note: all methods throw errors if the dataset has already been explicitly destroyed

#### Methods

- `toString()` : string
- `close()` : void
- `getGCPCount()` : int
- `getGCPProjection()` : string
- `getGCPs()` : object[]
- `setGCPs()` : void *(throws)*
- `executeSQL(string statement, Geometry spatial_filter = null, string dialect = null)` : [Layer](layer.md)
- `getFileList()` : string[]
	+ *In GDAL versions < 2.0 it will return an empty array for vector datasets*
- `flush()` : void
- `getMetadata(string domain = null)` : object

#### Properties

- `description` : string
- `rasterSize` : object
	- `x` : integer
	- `y` : integer
- `srs` : [SpatialReference](spatialreference.md) *(settable)*
- `driver` : [Driver](driver.md)
- `geoTransform` : Number[] *(settable)*
- `bands` : DatasetBands
- `layers` : DatasetLayers

#### Bands

- `bands.get(int band_id)` : [RasterBand](rasterband.md) *(throws, 1 based)*
- `bands.count()` : int
- `bands.forEach(function iterator)` : void

#### Layers

- `layers.get(int layer_id)` : [Layer](layer.md) *(throws)*
- `layers.get(string name)` : [Layer](layer.md) *(throws)*
- `layers.count()` : int
- `layers.create(string name, SpatialReference srs = null, OGRwkbGeometryType type = wkbUnknown, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.create(string name, SpatialReference srs = null, Function geometry_constructor, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.copy (string src_lyr_name, string dst_lyr_name, string[] options = null)` : [Layer](layer.md) *(throws)*
- `layers.remove(int layer_id)` : void *(throws)*
- `layers.forEach(function iterator)` : void
