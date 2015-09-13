'use strict'

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

if (process.env.TARGET !== 'SHARED') {
	describe.skip('Open', function() {

		describe('BIGTIFF', function() {
			var filename, ds;

			it('should not throw', function() {
				filename = path.join(__dirname,"data/sample_bigtiff.tif");
				ds = gdal.open(filename);
			});

			it('should be able to read raster size', function() {
				assert.equal(ds.rasterSize.x,100000);
				assert.equal(ds.rasterSize.y,100000);
				assert.equal(ds.bands.count(),1);
			});
		});
	});
};