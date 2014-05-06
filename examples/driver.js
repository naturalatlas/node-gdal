var gdal = require('../lib/index.js');

var driver = gdal.getDriverByName("GTiff");
var metadata = driver.getMetadata();

console.log(metadata);