var gdal = require('../lib/gdal.js');
var path = require('path');
var filename = path.resolve(__dirname, '../test/data/sample.tif');
var dataset = gdal.open(filename);

console.log('number of bands:', dataset.bands.count());
console.log('width:', dataset.rasterSize.x);
console.log('height:', dataset.rasterSize.y);
console.log('geotransform:', JSON.stringify(dataset.geoTransform));

for (var i = 0, n = dataset.bands.count(); i < n; i++) {
	var band = dataset.bands.get(1 + i);
	console.log('band[' + i + '] statistics: ', JSON.stringify(band.getStatistics(true, true)));
}
