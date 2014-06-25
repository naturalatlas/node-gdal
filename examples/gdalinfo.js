var gdal = require('../lib/gdal');
var util = require('util');

var filename = process.argv[2];
if (!filename) {
	console.error('Filename must be provided');
	process.exit(1);
}

var ds     = gdal.open(filename);

console.log('Driver: ' + ds.driver.description);

//-- raster dimensions --
var size = ds.rasterSize;
console.log('Size is ' + size.x + ', ' + size.y);

//-- spatial reference --
console.log('Coordinate System is: ');
console.log(ds.srs.toPrettyWKT());

//-- geotransform --
var geotransform = ds.geoTransform;
console.log('Origin = ('+geotransform[0]+', '+geotransform[3]+')');
console.log('Pixel Size = ('+geotransform[1]+', '+geotransform[5]+')');
console.log('GeoTransform =');
console.log(geotransform);

//-- corners --
var corners = {
	'Upper Left  ': {x: 0,        y: 0},
	'Upper Right ': {x: size.x,   y: 0},
	'Bottom Right': {x: size.x,   y: size.y},
	'Bottom Left ': {x: 0,        y: size.y},
	'Center      ': {x: size.x/2, y: size.y/2}
};

var wgs84 = gdal.SpatialReference.fromEPSG(4326);
var coord_transform = new gdal.CoordinateTransformation(ds.srs, wgs84);

console.log("Corner Coordinates:")
var corner_names = Object.keys(corners);
corner_names.forEach(function(corner_name){
	//convert pixel x,y to the coordinate system of the raster
	//then transform it to WGS84
	var corner        = corners[corner_name];
	var pt_orig       = {
		x: geotransform[0] + corner.x*geotransform[1] + corner.y*geotransform[2],
		y: geotransform[3] + corner.x*geotransform[4] + corner.y*geotransform[5]
	}
	var pt_wgs84      = coord_transform.transformPoint(pt_orig);
	var description   = util.format('%s (%d, %d) (%s, %s)',
	                        corner_name,
	                        Math.floor(pt_orig.x*100)/100,
	                        Math.floor(pt_orig.y*100)/100,
	                        gdal.decToDMS(pt_wgs84.x, 'Long'),
	                        gdal.decToDMS(pt_wgs84.y, 'Lat')
                        );
	console.log(description);
});
