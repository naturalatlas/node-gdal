## FeatureDefn

Note: all feature definitions owned by layers are cloned to avoid segfaults when layers are destroyed and to avoid modifying read-only feature definitions.

#### Constructor

- `FeatureDefn()`

#### Methods

- `toString()` : string
- `clone()` : [FeatureDefn](featuredefn.md)

#### Properties

- `name` : string
- `geomType` : [wkbGeometryType](constants.md#geometry-types)
- `geomIgnored` : bool
- `styleIgnored` : bool
- `fields` : [FeatureDefnFields](#fields)

#### Fields

- `fields.count()` : int
- `fields.add(FieldDefn def, bool approx = true)` : void
- `fields.remove(int i)` : void *(throws)*
- `fields.remove(string field_name)` : void *(throws)*
- `fields.get(int i)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.get(string field_name)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.getNames()` : string[]
- `fields.indexOf(string field_name)` : int
- `fields.reorder(int[] map)` : void *(throws)*