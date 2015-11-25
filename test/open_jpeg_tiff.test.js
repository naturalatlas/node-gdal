var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

if (process.env.TARGET !== 'SHARED') {
	describe('Open', function() {
		describe('JPEG-compressed TIFF', function() {
			var filename, ds;

			it('should not throw', function() {
				filename = path.join(__dirname, 'data/sample_jpeg.tif');
				ds = gdal.open(filename);
			});

			it('should be able to read raster size', function() {
				assert.equal(ds.rasterSize.x, 4);
				assert.equal(ds.rasterSize.y, 4);
				assert.equal(ds.bands.count(), 3);
			});
		});
	});
}
