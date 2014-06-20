## FieldDefn

Note: all field definitions owned by feature definitions are cloned to avoid segfaults when feature definitions are destroyed and to avoid modifying read-only field definitions.

#### Constructor

- `FieldDefn(string field_name, OGRFieldType type)`

#### Methods

- `toString()` : string
- `getName()` : string
- `setName(string name)` : void
- `getType()` : int
- `setType(OGRFieldType type)` : void
- `getJustify()` : int
- `setJustify(int justification)` : void
- `getWidth()` : int
- `setWidth(int width)` : void
- `getPrecision()` : int
- `setPrecision(int precision)` : void
- `isIgnored()` : bool
- `setIgnored(bool ignored)` : void