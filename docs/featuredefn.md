## FeatureDefn

Note: all feature definitions owned by layers are cloned to avoid segfaults when layers are destroyed and to avoid modifying read-only feature definitions.

#### Methods

- `toString()` : string
- `getName()` : string
- `getGeomType()` : int
- `setGeomType(wkbGeometryType type)` : void
- `clone()` : [FeatureDefn](featuredefn.md)
- `isGeometryIgnored()` : bool
- `setGeometryIgnored(bool ignore)` : void
- `isStyleIgnored()` : bool
- `setStyleIgnored(bool ignore)` : void

#### Properties

- `fields` : FeatureDefnFields

#### Fields

- `fields.count()` : int
- `fields.add(FieldDefn def, bool approx = true)` : void *(throws)*
- `fields.remove(int i)` : void *(throws)*
- `fields.remove(string field_name)` : void *(throws)*
- `fields.get(int i)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.get(string field_name)` : [FieldDefn](fielddefn.md) *(throws)*
- `fields.getNames()` : string[]
- `fields.indexOf(string field_name)` : int
- `fields.reorder(int[] map)` : void *(throws)*