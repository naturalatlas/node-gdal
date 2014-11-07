## Layer

Note: all methods throw errors if the layer has been destroyed by the datasource

#### Methods 

- `toString()` : string
- `testCapability(string capability_name)` : bool *([see capabilities](constants.md#vector-layer-capabilities))*
- `flush() *(throws)*`
- `getExtent(boolean force = true)` : [Envelope](envelope.md) *(throws)*
- `getSpatialFilter()` : [Geometry](geometry.md)
- `setSpatialFilter(Geometry filter)` : void
- `setSpatialFilter(Number minX, Number minY, Number maxX, Number maxY)` : void
- `setAttributeFilter(string filter)` : void *(throws)*

#### Properties

- `ds` : Datasource
- `srs` : [SpatialReference](spatialreference.md) //get
- `name` : string
- `geomType` : int *(wkbGeometryType)*
- `geomColumn` : string
- `fidColumn` : string
- `features` : [LayerFeatures](#features)
- `fields` : [LayerFields](#fields)

#### Features

- `features.add(feature f)` : [Feature](feature.md)
- `features.first()` : [Feature](feature.md)
- `features.next()` : [Feature](feature.md)
- `features.get(int fid)` : [Feature](feature.md)
- `features.count(bool force = true)` : int
- `features.set(feature f)` : void *(throws)*
- `features.set(int fid, feature f)` : void *(throws)*
- `features.remove(int fid)` : void *(throws)*
- `features.forEach(function iterator)` : void

#### Fields

- `fields.count()` : int
- `fields.add(FieldDefn def, bool approx = true)` : void *(throws)*
- `fields.add(FieldDefn[] defs, bool approx = true)` : void *(throws)*
- `fields.fromJSON(object json)` : void *(throws)*
- `fields.remove(int i)` : void *(throws)*
- `fields.remove(string field_name)` : void *(throws)*
- `fields.get(int i)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.get(string field_name)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.getNames()` : string[]
- `fields.indexOf(string field_name)` : int
- `fields.reorder(int[] map)` : void *(throws)*