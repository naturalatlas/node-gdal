## LineString

Inherits all methods and properties from [Geometry](geometry.md)

#### Constructor

- `LineString()`

#### Methods

- `getLength()` : Number
- `value(Number distance)` : [Point](point.md)

#### Properties

- `points` : LineStringPoints

#### Points

- `points.count()` : integer
- `points.add(Point pt)` : void
- `points.add(object pt)` : void
- `points.add(double x, y, z = 0)` : void
- `points.set(integer i, Point pt)` : void *(throws)*
- `points.set(integer i, object pt)` : void *(throws)*
- `points.set(integer i, double x, y, z = 0)` : void *(throws)*
- `points.get(integer i)` : [Point](point.md) *(throws)*
- `points.resize(integer pt_count)` : void
- `points.reverse()` : void
- `points.forEach(function iterator)` : void
- `points.concat(Points[] array)` : LineStringPoints
- `points.toArray()` : [Point](point.md)[]
