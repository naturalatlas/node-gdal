## Geometry

Note: all geometry objects owned by features are cloned to avoid segfaults when features are destroyed and to avoid modifying read-only geometries.

#### Static methods

- `getConstructor(wkbGeometryType type)` : Function
- `typeToName(wkbGeometryType type)` : string
- `fromWKBType(wkbGeometryType type)` : [Geometry](geometry.md)
- `fromWKT(string wkt, SpatialReference srs = null)` : [Geometry](geometry.md)
- `fromWKB(Buffer data, SpatialReference srs = null)` : [Geometry](geometry.md)

#### Static Properties

- `wkbType` : integer

#### Methods

- `toString()` : string
- `toKML()` : string
- `toGML()` : string
- `toJSON()` : string
- `toWKT()` : string
- `toWKB(string byte_order = 'MSB', string variant = 'OGC')` : Buffer
- `empty()` : void
- `isEmpty()` : boolean
- `isValid()` : boolean
- `isSimple()` : boolean
- `isRing()` : boolean
- `clone()` : [Geometry](geometry.md)
- `closeRings()`
- `intersects(Geometry geom)` : boolean
- `equals(Geometry geom)` : boolean
- `disjoint(Geometry geom)` : boolean
- `touches(Geometry geom)` : boolean
- `crosses(Geometry geom)` : boolean
- `within(Geometry geom)` : boolean
- `contains(Geometry geom)` : boolean
- `overlaps(Geometry geom)` : boolean
- `boundary()` : [Geometry](geometry.md)
- `distance(Geometry geom)` : Number
- `convexHull()` : [Geometry](geometry.md)
- `buffer(Number distance, int segs = 30)` : [Geometry](geometry.md)
- `intersection(Geometry geom)` : [Geometry](geometry.md)
- `union(Geometry geom)` : [Geometry](geometry.md)
- `difference(Geometry geom)` : [Geometry](geometry.md)
- `symDifference(Geometry geom)` : [Geometry](geometry.md)
- `centroid()` : [Point](point.md)
- `simplify(Number tolerance)` : void
- `simplifyPreserveTopology(Number tolerance)` : void
- `segmentize(Number segment_length)` : void
- `swapXY()` : void
- `getEnvelope()` : [Envelope](envelope.md) 
- `getEnvelope3D()` : [Envelope3D](envelope3d.md) 
- `transform(CoordinateTransformation transform)` : void *(throws)*
- `transformTo(SpatialReference srs)` : void *(throws)*

#### Properties 

- `srs` : [SpatialReference](spatialreference.md) //get and set
- `wkbType` : integer // wkbGeometryType
- `name` : string
- `wkbSize` : integer
- `dimension` : integer
- `coordinateDimension` : integer