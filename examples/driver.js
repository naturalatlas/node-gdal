var gdal = require('../lib/gdal.js');

var driver = gdal.drivers.get('GTiff');
var metadata = driver.getMetadata();

console.log(metadata);