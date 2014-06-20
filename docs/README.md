## Node.js GDAL Bindings

#### Methods

- `open(string name, string mode = 'r', string[] drivers = null, string[] options = null)` : [Dataset](dataset.md)
- `open(string name, string mode = 'r', string[] drivers = null, int x_size = 0, int y_size = 0, int n_bands = 0, int datatype = 0, string[] options = null)` : [Dataset](dataset.md) 
- `close(Dataset ds)` : void
- `getConfigOption(string name)` : string
- `setConfigOption(string name, string value)` : void
- `quiet(string name, string value)` : void
- `verbose(string name, string value)` : void

#### Drivers 

- `drivers.count()` : int
- `drivers.get(int id)` : [Driver](driver.md) *(throws)*
- `drivers.get(string name)` : [Driver](driver.md) *(throws)*
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