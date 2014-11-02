## CoordinateTransformation

#### Constructor

- `CoordinateTransformation(SpatialReference source, SpatialReference target)`
- `CoordinateTransformation(SpatialReference source, Dataset target)`
	+ *converts to pixel coordinates*

#### Methods

- `toString()` : string
- `transformPoint(obj point)` : object *(throws)*
- `transformPoint(Number x, Number y, Number z = 0)` : object *(throws)*