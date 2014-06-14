# Node.js OGR Bindings

## Globals

#### Methods

- `open(string ds_name, boolean update = false) : Datasource #throws`
- `getDriverByName(string name) : Driver #throws`
- `getDriverCount() : integer`
- `getDriver(integer driver_index) : Driver #throws`
- `getOpenDSCount() : integer`
- `getOpenDS(integer ds_index) : Datasource`

#### Properties

- `drivers : string[]`

## SpatialReference

Note: all spatial references owned by features, layers, etc are cloned to avoid segfaults when layers are destroyed and to avoid modifying read-only spatial references. They are only cloned once. This means that any method wrapping a method returning an OGRSpatialReference pointer will always return the same javascript object - instead of recloning. 

#### Constructors 

- `SpatialReference()`
- `SpatialReference(string wkt) #throws`

#### Static Methods

- `fromWKT(string wkt) : SpatialReference #throws`
- `fromProj4(string proj4) : SpatialReference #throws`
- `fromEPSG(integer code) : SpatialReference #throws`
- `fromEPSGA(integer code) : SpatialReference #throws`
- `fromWMSAUTO(string wmsauto) : SpatialReference #throws`
- `fromXML(string xml) : SpatialReference  #throws`
- `fromURN(string urn) : SpatialReference #throws`
- `fromCRSURL(string url) : SpatialReference #throws`
- `fromURL(string url) : SpatialReference #throws`
- `fromMICoordSys(string name) : SpatialReference #throws`
- `fromUserInput(string name) : SpatialReference #throws`

#### Methods 

- `toString() : string`
- `toWKT() : string #throws`
- `toPrettyWKT(boolean simplify = false) : string #throws`
- `toProj4() : string #throws`
- `toXML() : string #throws`
- `clone() : SpatialReference`
- `cloneGeogCS() : SpatialReference`
- `setWellKnownGeogCS(string name) : void #throws`
- `autoIdentifyEPSG() : void #throws`
- `morphToESRI() : void #throws`
- `morphFromESRI() : void #throws`
- `EPSGTreatsAsLatLong() : boolean`
- `EPSGTreatsAsNorthingEasting() : boolean`
- `getLinearUnits() : Number`
- `getAngularUnits() : Number`
- `isGeocentric() : boolean`
- `isProjected() : boolean`
- `isLocal() : boolean`
- `isVertical() : boolean`
- `isCompound() : boolean`
- `isSameGeogCS(SpatialReference srs) : boolean`
- `isSameVertCS(SpatialReference srs) : boolean`
- `isSame(SpatialReference srs) : boolean`
- `getAuthorityName(string key) : string`
- `getAuthorityCode(string key) : string`
- `getAttrValue(string node_name, integer attr_index = 0) : string`

## Datasource

Note: all methods throw errors if the datasource has already been explicitly destroyed

#### Methods 

- `destroy() : void`
- `toString() : string`
- `getName() : string`
- `getDriver() : Driver`
- `testCapability(string capability_name) : boolean`
- `executeSQL(string sql_text, Geometry spatial_filter = null, string dialect = null) : Layer #throws // result sets are released when datasource is destroyed or V8 does GC on the layer`
- `syncToDisk() #throws`

#### Properties

- `layers : DatasetLayers`

## DatasetLayers 

#### Methods

- `get(integer layer_id) : Layer #throws`
- `get(string name) : Layer #throws`
- `count() : Integer`
- `create(string name, SpatialReference srs = null, OGRwkbGeometryType type = wkbUnknown, string[] options = null) : Layer #throws`
- `copy (string src_lyr_name, string dst_lyr_name, string[] options = null) : Layer #throws`
- `remove (int layer_id) : void #throws`
- `forEach(function iterator) : void`

#### Properties

- `ds : Datasource`

## Layer

Note: all methods throw errors if the layer has been destroyed by the datasource

#### Methods 

- `toString() : string`
- `getGeomType() : integer`
- `getName() : string`
- `testCapability(string capability_name) : boolean`
- `syncToDisk() #throws`
- `getFIDColumn() : string`
- `getGeometryColumn() : string`

#### Properties

- `ds : Datasource`
- `srs : SpatialReference //get`
- `features : LayerFeatures`
- `fields : FeatureDefnFields`

## LayerFeatures

#### Methods

- `add(feature f) : Feature`
- `first() : Feature`
- `next() : Feature`
- `get(integer fid) : Feature`
- `count(boolean force = true) : integer`
- `set(feature f) : void #throws`
- `set(integer fid, feature f) : void #throws`
- `remove(integer fid) : void #throws`
- `forEach(function iterator) : void`

#### Properties

- `layer : Layer`

## FeatureDefnFields

#### Methods

- `count() : integer`
- `add(FieldDefn def, bool approx = true) : void #throws`
- `remove(integer i) : void #throws`
- `remove(string field_name) : void #throws`
- `get(integer i) : FieldDefn #throws`
- `get(string field_name) : FieldDefn #throws`
- `getNames() : string[]`
- `indexOf(string field_name) : integer`
- `reorder(integer[] map) : void #throws`

## Driver

#### Methods 

- `toString() : string`
- `getName() : string`
- `open(string ds_name, boolean update = false) : Datasource #throws`
- `testCapability(string capability_name) : boolean`
- `createDataSource(string name, string[] dsco = null) : Datasource #throws`
- `deleteDataSource() : Datasource #throws`
- `copyDataSource(Datasource src, string new_name, string[] dsco = null) : Datasource #throws`

## Geometry

Note: all geometry objects owned by features are cloned to avoid segfaults when features are destroyed and to avoid modifying read-only geometries.

#### Static methods

- `create(wkbGeometryType type) : Geometry`
- `createFromWKT(string wkt, SpatialReference srs = null) : Geometry`

#### Methods

- `toString() : string`
- `getDimension() : integer`
- `getCoordinateDimension() : integer`
- `isEmpty() : boolean`
- `isValid() : boolean`
- `isSimple() : boolean`
- `isRing() : boolean`
- `clone() : Geometry`
- `empty() : void`
- `wkbSize() : integer`
- `getGeometryType() : integer`
- `getGeometryName() : string`
- `exportToKML() : string`
- `exportToGML() : string`
- `exportToJSON() : string`
- `exportToWKT() : string`
- `closeRings()`
- `intersects(Geometry geom) : boolean`
- `equals(Geometry geom) : boolean`
- `disjoint(Geometry geom) : boolean`
- `touches(Geometry geom) : boolean`
- `crosses(Geometry geom) : boolean`
- `within(Geometry geom) : boolean`
- `contains(Geometry geom) : boolean`
- `overlaps(Geometry geom) : boolean`
- `boundary() : Geometry`
- `distance(Geometry geom) : Number`
- `convexHull() : Geometry`
- `buffer(Number distance, int segs = 30) : Geometry`
- `intersection(Geometry geom) : Geometry`
- `union(Geometry geom) : Geometry`
- `difference(Geometry geom) : Geometry`
- `symDifference(Geometry geom) : Geometry`
- `centroid() : Point`
- `simplify(Number tolerance) : void`
- `simplifyPreserveTopology(Number tolerance) : void`
- `segmentize(Number segment_length) : void`
- `swapXY() : void`
- `getEnvelope() : object`
- `getEnvelope3D() : object`
- `transform(CoordinateTransformation transform) : void #throws`
- `transformTo(SpatialReference srs) : void #throws`

#### Properties 

- `srs : SpatialReference //get and set`

## Point 

Inherits from Geometry

#### Constructors

- `Point()`
- `Point(Number x, Number y)`
- `Point(Number x, Number y, Number z)`

#### Properties

- `x : Number`
- `y : Number`
- `z : Number`



## LineString

Inherits from Geometry

#### Constructor

- `LineString()`

#### Methods

- `getPoint(integer i) : Geometry`
- `addPoint(Point geom) : void`
- `getNumPoints() : integer`
- `getLength() : Number`
- `value(Number distance) : Point`



## LinearRing

Inherits from LineString

#### Constructor

- `LinearRing()`

#### Methods

- `getArea() : Number`


## Polygon

Inherits from Geometry

#### Constructor

- `Polygon()`

#### Methods

- `getArea() : Number`
- `getNumInteriorRings() : integer`
- `addRing(LinearRing ring) : void`
- `getExteriorRing() : LinearRing`
- `getInteriorRing(integer index) : LinearRing`



## GeometryCollection

Inherits from Geometry

#### Constructor

- `GeometryCollection()`

#### Methods

- `addGeometry(Geometry geom) : void #throws`
- `getGeometry(integer i) : Geometry`
- `getNumGeometries() : integer`
- `getArea() : Number`
- `getLength() : Number`



## MultiPoint

Inherits from GeometryCollection

#### Constructor

- `MultiPoint()`

#### Methods

- `getGeometry(integer i) : Point`


## MultiLineString

Inherits from GeometryCollection

#### Constructor

- `MultiLineString()`

#### Methods

- `polygonize() : void #throws`
- `getGeometry(integer i) : LineString`



## MultiPolygon

Inherits from GeometryCollection

#### Constructor

- `MultiPolygon()`

#### Methods

- `unionCascaded(Geometry geom) : Geometry #throws`
- `getGeometry(integer i) : Polygon`


## Feature

Note: all methods throw errors if the feature has already been explicitly destroyed

#### Constructor

- `Feature(FeatureDefn defn)`

#### Methods

- `toString() : string`
- `getDefn() : FeatureDefn`
- `getGeometry() : Geometry #throws`
- `setGeometryDirectly(Geometry geom) : void #throws //alias to setGeometry in current version` 
- `setGeometry(Geometry geom) : void #throws`
- `clone() : Feature`
- `equal(Feature f) : boolean`
- `getFieldDefn(integer index) : FieldDefn #throws`
- `getFID() : integer`
- `setFID(integer fid) : void`
- `setFrom(Feature f, boolean forgiving = true) : void #throws`
- `setFrom(Feature f, integer[] index_map, boolean forgiving = true) : void #throws`

#### Properties

- `fields : FeatureFields`

## FeatureFields

#### Methods 

- `count() : integer`
- `get(integer i) : value`
- `get(string name) : value`
- `getNames() : string[]`
- `set(integer i, value) : integer`
- `set(Array values) : integer`
- `set(object values) : integer`
- `reset(object values) : integer`
- `indexOf(string field_name) : integer`
- `toJSON() : object`
- `toArray() : Array`
- `forEach(function iterator) : void`

#### Properties

- `feature : Feature`

## FieldDefn

Note: all field definitions owned by feature definitions are cloned to avoid segfaults when feature definitions are destroyed and to avoid modifying read-only field definitions.

#### Constructor

- `FieldDefn(string field_name, OGRFieldType type)`

#### Methods

- `toString() : string`
- `getName() : string`
- `setName(string name) : void`
- `getType() : integer`
- `setType(OGRFieldType type) : void`
- `getJustify() : integer`
- `setJustify(integer justification) : void`
- `getWidth() : integer`
- `setWidth(integer width) : void`
- `getPrecision() : integer`
- `setPrecision(integer precision) : void`
- `isIgnored() : boolean`
- `setIgnored(boolean ignored) : void`


## FeatureDefn

Note: all feature definitions owned by layers are cloned to avoid segfaults when layers are destroyed and to avoid modifying read-only feature definitions.

#### Methods

- `toString() : string`
- `getName() : string`
- `getGeomType() : integer`
- `setGeomType(wkbGeometryType type) : void`
- `clone() : FeatureDefn`
- `isGeometryIgnored() : boolean`
- `setGeometryIgnored(boolean ignore) : void`
- `isStyleIgnored() : boolean`
- `setStyleIgnored(boolean ignore) : void`

#### Properties

- `fields : FeatureDefnFields`

## CoordinateTransformation

#### Constructor

- `CoordinateTransformation(SpatialReference source, SpatialReference target)`

#### Methods

- `toString() : string`
- `transformPoint(obj point) : object #throws`
- `transformPoint(Number x, Number y, Number z = 0) : object #throws`
