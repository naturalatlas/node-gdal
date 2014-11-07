## FieldDefn

Note: all field definitions owned by feature definitions are cloned to avoid segfaults when feature definitions are destroyed and to avoid modifying read-only field definitions.

#### Constructor

- `FieldDefn(string field_name, OGRFieldType type)` 
    - type: [OGRFieldType](constants.md#field-types)

#### Methods

- `toString()` : string

#### Properties

- `name` : string
- `type` : [OGRFieldType](constants.md#field-types)
- `width` : int
- `precision` : int
- `justification` : [OGRJustification](constants.md#field-justification)
- `ignored` : bool