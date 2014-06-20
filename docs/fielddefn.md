## FieldDefn

Note: all field definitions owned by feature definitions are cloned to avoid segfaults when feature definitions are destroyed and to avoid modifying read-only field definitions.

#### Constructor

- `FieldDefn(string field_name, OGRFieldType type)`

#### Methods

- `toString()` : string

#### Properties

- `name` : string
- `type` : int *([OGRFieldType](http://www.gdal.org/ogr__core_8h.html#a787194bea637faf12d61643124a7c9fc))*
- `width` : int
- `precision` : int
- `justification` : int *(OGRJustification)*
    + OJUndefined = 0
    + OJLeft = 1
    + OJRight = 2
- `ignored` : bool