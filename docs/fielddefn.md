## FieldDefn

Note: all field definitions owned by feature definitions are cloned to avoid segfaults when feature definitions are destroyed and to avoid modifying read-only field definitions.

#### Constructor

- `FieldDefn(string field_name, string type)`

#### Methods

- `toString()` : string

#### Properties

- `name` : string
- `type` : string
    + `'integer'`
    + `'integer[]'`
    + `'real'`
    + `'real[]'`
    + `'string'`
    + `'string[]'`
    + `'binary'`
    + `'date'`
    + `'time'`
    + `'dateTime'`
- `width` : int
- `precision` : int
- `justification` : string
    + `'Left'`
    + `'Right'`
- `ignored` : bool