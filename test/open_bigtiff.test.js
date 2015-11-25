var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

if (process.env.TARGET !== 'SHARED') {
	describe('Open', function() {
		describe('BIGTIFF', function() {
			var filename, ds;

			// https://github.com/naturalatlas/node-gdal/issues/139
			// > tifftopnm sample.tif | pamscale -width=100000 -height=1 | pnmtotiff > sample_wide.tif
			// > tiffcp -c zip -8 sample_wide.tif sample_bigtiff.tif

			it('should not throw', function() {
				filename = path.join(__dirname, 'data/sample_bigtiff.tif');
				ds = gdal.open(filename);
			});

			it('should be able to read raster size', function() {
				assert.equal(ds.rasterSize.x, 100000);
				assert.equal(ds.rasterSize.y, 1);
				assert.equal(ds.bands.count(), 1);
			});
		});
	});
}
