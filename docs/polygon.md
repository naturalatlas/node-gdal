## Polygon

Inherits all methods and properties from [Geometry](geometry.md)

#### Constructor

- `Polygon()`

#### Methods

- `getArea()` : Number

#### Properties

- `rings` : [PolygonRings](#rings)

#### Rings

- `rings.get(integer i)` : [LinearRing](linearring.md)
- `rings.add(LinearRing ring)` : void
- `rings.add(LinearRing[] rings)` : void
- `rings.count()` : integer
- `rings.forEach(function iterator)` : void
- `rings.toArray()` : [LinearRing](linearring.md)[]
