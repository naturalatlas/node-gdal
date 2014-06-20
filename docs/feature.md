## Feature

Note: all methods throw errors if the feature has already been explicitly destroyed

#### Constructor

- `Feature(FeatureDefn defn)`

#### Methods

- `toString()` : string
- `getGeometry()` : [Geometry](geometry.md) *(throws)*
- `setGeometry(Geometry geom)` : void *(throws)*
- `setGeometryDirectly(Geometry geom)` : void *(throws, alias to setGeometry currently)*
- `clone()` : [Feature](feature.md)
- `equal(Feature f)` : bool
- `getFieldDefn(int index)` : [FieldDefn](fielddefn.md) *(throws)*
- `setFrom(Feature f, bool forgiving = true)` : void *(throws)*
- `setFrom(Feature f, int[] index_map, bool forgiving = true)` : void *(throws)*

#### Properties


- `fid` : int *(R/W)*
- `defn` : [FeatureDefn](featuredefn.md)
- `fields` : FeatureFields

#### Fields

- `fields.count()` : int
- `fields.get(int i)` : value
- `fields.get(string name)` : value
- `fields.getNames()` : string[]
- `fields.set(int i, value)` : int
- `fields.set(Array values)` : int
- `fields.set(object values)` : int
- `fields.reset(object values)` : int
- `fields.indexOf(string field_name)` : int
- `fields.toJSON()` : object
- `fields.toArray()` : Array
- `fields.forEach(function iterator)` : void