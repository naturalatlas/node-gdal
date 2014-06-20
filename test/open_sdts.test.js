'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe.skip('SDTS (DDF)', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/sdts/8821CATD.DDF");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.rasterSize.x,343);
			assert.equal(ds.rasterSize.y,471);
			assert.equal(ds.bands.count(),1);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [
				658995,
				30,
				0,
				4929375,
				0,
				-30
			];

			var actual_geotransform = ds.geoTransform;
			var delta = .00001;
			assert.closeTo(actual_geotransform[0], expected_geotransform[0], delta);
			assert.closeTo(actual_geotransform[1], expected_geotransform[1], delta);
			assert.closeTo(actual_geotransform[2], expected_geotransform[2], delta);
			assert.closeTo(actual_geotransform[3], expected_geotransform[3], delta);
			assert.closeTo(actual_geotransform[4], expected_geotransform[4], delta);
			assert.closeTo(actual_geotransform[5], expected_geotransform[5], delta);
		});

		it('should have projection', function() {
			assert.match(ds.srs.toWKT(), /PROJCS/);
		});

		it('should be able to read statistics', function() {
			var band = ds.bands.get(1);
			var expected_stats = {
				min: 1520,
				max: 1826,
				mean: 1625.9093194071836,
				std_dev: 48.585305943514605
			};

			var actual_stats = band.getStatistics(false, true);
			var delta = .00001;
			assert.closeTo(expected_stats.min, actual_stats.min, delta);
			assert.closeTo(expected_stats.max, actual_stats.max, delta);
			assert.closeTo(expected_stats.mean, actual_stats.mean, delta);
			assert.closeTo(expected_stats.std_dev, actual_stats.std_dev, delta);
		});
	});
});