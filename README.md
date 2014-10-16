# node-gdal
[![NPM version](http://img.shields.io/npm/v/gdal.svg)](https://www.npmjs.org/package/gdal)
[![Installs](http://img.shields.io/npm/dm/gdal.svg)](https://www.npmjs.org/package/gdal)
[![Build Status](http://img.shields.io/travis/naturalatlas/node-gdal/master.svg)](https://travis-ci.org/naturalatlas/node-gdal)
[![Build Status](https://ci.appveyor.com/api/projects/status/mo06c2r5opdwak95)](https://ci.appveyor.com/project/brianreavis/node-gdal)

Read and write raster and vector geospatial datasets straight from [Node.js](http://nodejs.org) with this native [GDAL](http://www.gdal.org/) binding. GDAL [1.11.0](http://trac.osgeo.org/gdal/wiki/Release/1.11.0-News) ([GEOS](http://trac.osgeo.org/geos/) [3.4.2](http://trac.osgeo.org/geos/browser/tags/3.4.2/NEWS), [Proj.4](http://trac.osgeo.org/proj/) [4.8.0](http://www.osgeo.org/node/1268)) comes bundled, so node-gdal will work straight out of the box.

```sh
$ npm install gdal --save
```

To link against shared libgdal, install using:

```sh
# requires libgdal-dev (debian: sudo apt-get install libgdal-dev)
$ npm install gdal --build-from-source --shared_gdal
```

### Links

- [**Documentation**](docs/)
    - [Dataset](docs/dataset.md)
    - [Layer](docs/layer.md)
    - [RasterBand](docs/rasterband.md)
    - [Feature](docs/feature.md)
    - [FeatureDefn](docs/featuredefn.md)
    - [FieldDefn](docs/fielddefn.md)
    - [Geometry](docs/geometry.md)
        - [Point](docs/point.md)
        - [LineString](docs/linestring.md)
        - [LinearRing](docs/linearring.md)
        - [Polygon](docs/polygon.md)
        - [GeometryCollection](docs/geometrycollection.md)
        - [MultiPoint](docs/multipoint.md)
        - [MultiLineString](docs/multilinestring.md)
        - [MultiPolygon](docs/multipolygon.md)
    - [SpatialReference](docs/spatialreference.md)
    - [CoordinateTransformation](docs/coordinatetransformation.md)
    - [Driver](docs/driver.md)
    - [Envelope](docs/envelope.md)
    - [Envelope3D](docs/envelope3d.md)
- [**Examples**](examples/)

### Sample Usage

##### Raster
```js
var gdal = require("gdal");
var dataset = gdal.open("sample.tif");

console.log("number of bands: " + dataset.bands.count());
console.log("width: " + dataset.rasterSize.x);
console.log("height: " + dataset.rasterSize.y);
console.log("geotransform: " + dataset.geoTransform);
console.log("srs: " + (ds.srs ? ds.srs.toWKT() : 'null'));
```
##### Vector
```js
var gdal = require("gdal");
var dataset = gdal.open("sample.shp");
var layer = dataset.layers.get(0);

console.log("number of features: " + layer.features.count());
console.log("fields: " + layer.fields.getNames());
console.log("extent: " + JSON.stringify(layer.extent));
console.log("srs: " + (layer.srs ? layer.srs.toWKT() : 'null'));
```

## Notes

- This binding is currently *not* async, so it will block node's event loop. This will be changing in the future ([#18](https://github.com/naturalatlas/node-gdal/issues/18#issuecomment-57513723)). In the meantime, be very careful (or avoid) using it in server code. 

## Contributors

This binding is a collaboration between [Natural Atlas](https://github.com/naturalatlas) and [Mapbox](https://github.com/mapbox). Its contributors are [Brandon Reavis](https://github.com/brandonreavis), [Brian Reavis](https://github.com/brianreavis), [Dane Springmeyer](https://github.com/springmeyer), [Zac McCormick](https://github.com/zhm), and [others](https://github.com/naturalatlas/node-gdal/graphs/contributors).

Before submitting pull requests, please update the [tests](test) and make sure they all pass.

```sh
$ make test # test against bundled gdal
$ make test-shared # test against shared gdal
```

## License

Copyright &copy; 2014 [Brandon Reavis](https://github.com/brandonreavis) & [Contributors](https://github.com/naturalatlas/node-gdal/graphs/contributors)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
