## SpatialReference

Note: all spatial references owned by features, layers, etc are cloned to avoid segfaults when layers are destroyed and to avoid modifying read-only spatial references. They are only cloned once. This means that any method wrapping a method returning an OGRSpatialReference pointer will always return the same javascript object - instead of recloning. 

#### Constructors 

- `SpatialReference()`
- `SpatialReference(string wkt)` *(throws)*

#### Static Methods

- `fromWKT(string wkt)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromProj4(string proj4)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromEPSG(int code)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromEPSGA(int code)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromWMSAUTO(string wmsauto)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromXML(string xml)` : [SpatialReference](spatialreference.md)  *(throws)*
- `fromURN(string urn)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromCRSURL(string url)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromURL(string url)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromMICoordSys(string name)` : [SpatialReference](spatialreference.md) *(throws)*
- `fromUserInput(string name)` : [SpatialReference](spatialreference.md) *(throws)*

#### Methods 

- `toString()` : string
- `toWKT()` : string *(throws)*
- `toPrettyWKT(bool simplify = false)` : string *(throws)*
- `toProj4()` : string *(throws)*
- `toXML()` : string *(throws)*
- `clone()` : [SpatialReference](spatialreference.md)
- `cloneGeogCS()` : [SpatialReference](spatialreference.md)
- `setWellKnownGeogCS(string name)` : void *(throws)*
- `autoIdentifyEPSG()` : void *(throws)*
- `morphToESRI()` : void *(throws)*
- `morphFromESRI()` : void *(throws)*
- `EPSGTreatsAsLatLong()` : bool
- `EPSGTreatsAsNorthingEasting()` : bool
- `getLinearUnits()` : Number
- `getAngularUnits()` : Number
- `isGeocentric()` : bool
- `isProjected()` : bool
- `isLocal()` : bool
- `isVertical()` : bool
- `isCompound()` : bool
- `isSameGeogCS(SpatialReference srs)` : bool
- `isSameVertCS(SpatialReference srs)` : bool
- `isSame(SpatialReference srs)` : bool
- `getAuthorityName(string key)` : string
- `getAuthorityCode(string key)` : string
- `getAttrValue(string node_name, int attr_index = 0)` : string