## GeometryCollection

Inherits all methods and properties from [Geometry](geometry.md)

#### Constructor

- `GeometryCollection()`

#### Methods

- `getArea()` : Number
- `getLength()` : Number

#### Properties

- `children` : GeometryCollectionChildren

#### Children

- `children.add(Geometry child)` : void *(throws)*
- `children.add(Geometry[] children)` : void *(throws)*
- `children.get(integer i)` : [Geometry](geometry.md)
- `children.count()` : integer
- `children.forEach(function iterator)` : void
- `children.toArray()` : Geometry[]
