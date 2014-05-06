var gdal = require('../lib/index.js');

var filename = __dirname + "/sample.tif";
var dataset  = gdal.open(filename);

console.log("number of bands: " + dataset.getRasterCount());
console.log("width: " + dataset.getRasterXSize());
console.log("height: " + dataset.getRasterYSize());
console.log("geotransform: ");
console.log(dataset.getGeoTransform());
console.log("");

var n_bands = dataset.getRasterCount();
for (var i = 0, n = n_bands; i < n; i++) {
	console.log("band[" + i + "] statistics: ");
	var band = dataset.getRasterBand(1 + i);
	console.log(band.getStatistics(0,1))
}
