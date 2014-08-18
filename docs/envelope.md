## Envelope

#### Constructor

- `Envelope()`
- `Envelope(Envelope envelope)`

#### Methods

- `isEmpty()` : boolean
- `contains(Envelope envelope)` : boolean
- `intersects(Envelope envelope)` : boolean
- `intersect(Envelope envelope)` : void
- `merge(Envelope envelope)` : void
- `merge(Number x, Number y)` : void
- `toPolygon()` : [Polygon](polygon.md)

#### Properties

- `minX` : Number
- `maxX` : Number
- `minY` : Number
- `maxY` : Number