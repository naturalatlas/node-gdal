## Polygon

Inherits all methods and properties from [Geometry](geometry.md)

#### Constructor

- `Polygon()`

#### Methods

- `getArea()` : Number

#### Properties

- `rings` : PolygonRings

#### Rings

- `rings.get(integer i)` : [LinearRing](linearring.md)
- `rings.add(LinearRing ring)` : [LinearRing](linearring.md)
- `rings.count()` : integer
- `rings.forEach(function iterator)` : void