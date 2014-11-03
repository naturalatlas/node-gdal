## Node.js GDAL Bindings

#### Methods

- `open(string name, string mode = 'r', string[] drivers = null, string[] options = null)` : [Dataset](dataset.md)
- `open(string name, string mode = 'r', string[] drivers = null, int x_size = 0, int y_size = 0, int n_bands = 0, GDALDatatype datatype = gdal.GDT_Byte, string[] options = null)` : [Dataset](dataset.md) 
- `quiet()` : void
- `verbose()` : void
- `decToDMS(number angle, string axis, int precision = 2)` : string
    + axis: `'lat'` or `'long'`
- `reprojectImage(object options)` *(throws)*
	+ src: [Dataset](dataset.md) *(required)*
	+ dst: [Dataset](dataset.md) *(required)*
	+ s_srs: [SpatialReference](spatialreference.md)
	+ t_srs: [SpatialReference](spatialreference.md)
	+ r: string *(resampling method)*
	+ cutline: [Geometry](geometry.md) 
		+ *Must be in src dataset pixel coordinates. Use [CoordinateTransformation](coordinatetransformation.md) to convert between georeferenced coordinates and pixel coordinates*
	+ blend: Number
		+ *Cutline blend distance in pixels*
	+ srcBands: int[]
	+ dstBands: int[]
	+ srcAlphaBand: int
	+ dstAlphaBand: int
	+ srcNodata: Number
	+ dstNodata: Number
	+ options: object | array
		+ *additional warp options*
- `fillNodata(object options)` *(throws)*
	+ src: [RasterBand](rasterband.md) *(updated in place)*
	+ mask: [RasterBand](rasterband.md) 
	+ searchDist: Number
	+ smoothingIterations: int
- `contourGenerate(object options)` *(throws)*
	+ src: [RasterBand](rasterband.md)
	+ dst: [Layer](layer.md)
	+ base: Number
	+ interval: Number
	+ fixedLevels: Number[]
	+ noData: Number
	+ idField: int
	+ elevField: int
- `polygonize(object options)` *(throws)*
	+ src: [RasterBand](rasterband.md)
	+ dst: [Layer](layer.md)
	+ mask: [RasterBand](rasterband.md)
	+ pixValField: int
		+ *the attribute field index indicating the feature attribute into which the pixel value of the polygon should be written.*
	+ connectedness: int
		+ *either `4` indicating that diagonal pixels are not considered directly adjacent for polygon membership purposes or `8` indicating they are.*
	+ useFloats: boolean
		+ use floating point buffers instead of int buffers
- `sieveFilter(object options)` *(throws)*
	+ src: [RasterBand](rasterband.md)
	+ dst: [RasterBand](rasterband.md)
		+ *the output raster band.  It may be the same as src band to update the source in place.*
	+ mask: [RasterBand](rasterband.md)
		+ *optional mask band.  All pixels in the mask band with a value other than zero will be considered suitable for inclusion in polygons.*
	+ threshold: Number
		+ *raster polygons with sizes smaller than this will be merged into their largest neighbour.*
	+ connectedness: int
		+ *either `4` indicating that diagonal pixels are not considered directly adjacent for polygon membership purposes or `8` indicating they are.*
- `checksumImage(RasterBand src, int x = 0, int y = 0, int w = src_w, int h = src_h)` *(throws)*

#### Configuration

- `config.get(string name)` : string
- `config.set(string name, string value)` : void

#### Drivers 

- `drivers.count()` : int
- `drivers.get(int id)` : [Driver](driver.md) *(throws)*
- `drivers.get(string name)` : [Driver](driver.md) *(throws)*
- `drivers.getNames()` : string[]
- `drivers.forEach(function iterator)` : void *(throws)*

#### Classes

- [Dataset](dataset.md)
- [Layer](layer.md)
- [RasterBand](rasterband.md)
- [Feature](feature.md)
- [FeatureDefn](featuredefn.md)
- [FieldDefn](fielddefn.md)
- [Geometry](geometry.md)
    - [Point](point.md)
    - [LineString](linestring.md)
    - [LinearRing](linearring.md)
    - [Polygon](polygon.md)
    - [GeometryCollection](geometrycollection.md)
    - [MultiPoint](multipoint.md)
    - [MultiLineString](multilinestring.md)
    - [MultiPolygon](multipolygon.md)
- [SpatialReference](spatialreference.md)
- [CoordinateTransformation](coordinatetransformation.md)
- [Driver](driver.md)
- [Envelope](envelope.md)
- [Envelope3D](envelope3d.md)

