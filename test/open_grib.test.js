var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	afterEach(gc);

	describe('Grib', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname, 'data/Pacific.wind.7days.grb');
			ds = gdal.open(filename);
		});
		it('should be able to read raster', function() {
			assert.equal(ds.rasterSize.x, 57, 'rasterSize.x');
			assert.equal(ds.rasterSize.y, 39, 'rasterSize.y');
			assert.equal(ds.bands.count(), 54, 'bands.count()');
		});
	});
});
