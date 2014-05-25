# node-gdal
[![NPM version](https://badge.fury.io/js/gdal.png)](http://badge.fury.io/js/gdal)
[![Build Status](https://travis-ci.org/naturalatlas/node-gdal.png?branch=master)](https://travis-ci.org/naturalatlas/node-gdal)
[![Build Status](https://ci.appveyor.com/api/projects/status/mo06c2r5opdwak95)](https://ci.appveyor.com/project/brianreavis/node-gdal)

Read and write raster geospatial files straight from [Node.js](http://nodejs.org) with this native [GDAL](http://www.gdal.org/) binding. GDAL [1.11.0](http://trac.osgeo.org/gdal/wiki/Release/1.11.0-News) comes bundled, so node-gdal will work straight out of the box.

```sh
$ npm install gdal --save
```

To link against shared libgdal, install using:

```sh
$ npm install --shared_gdal
```

## Usage

```js
var gdal = require('gdal');
var driver = gdal.getDriverByName("GTiff");
var dataset = gdal.open("./examples/sample.tif");

console.log("number of bands: " + dataset.getRasterCount());
console.log("width: " + dataset.getRasterXSize());
console.log("height: " + dataset.getRasterYSize());
console.log("geotransform: " + dataset.getGeoTransform());
```

## Test

```sh
$ npm test
```

## License

Copyright &copy; 2014 [Brandon Reavis](https://github.com/brandonreavis) & [Contributors](https://github.com/naturalatlas/node-gdal/graphs/contributors)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.