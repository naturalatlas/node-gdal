'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('VRT', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/sample.vrt");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.getRasterXSize(),984);
			assert.equal(ds.getRasterYSize(),804);
			assert.equal(ds.getRasterCount(),1);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [
				0,
				7.502071930146189,
				0,
				2485710.4658232867,
				0,
				-7.502071930145942
			];

			var actual_geotransform = ds.getGeoTransform();
			var delta = .00001;
			assert.closeTo(actual_geotransform[0], expected_geotransform[0], delta);
			assert.closeTo(actual_geotransform[1], expected_geotransform[1], delta);
			assert.closeTo(actual_geotransform[2], expected_geotransform[2], delta);
			assert.closeTo(actual_geotransform[3], expected_geotransform[3], delta);
			assert.closeTo(actual_geotransform[4], expected_geotransform[4], delta);
			assert.closeTo(actual_geotransform[5], expected_geotransform[5], delta);
		});

		it('should be able to read statistics', function() {
			var srs = ds.getProjectionRef();
			assert.ok(srs.indexOf('PROJCS') > -1);
			var band = ds.getRasterBand(1);
			var expected_stats = {
				min: 0,
				max: 100,
				mean: 29.725628716175223,
				std_dev: 36.98885954363488
			};

			var actual_stats = band.getStatistics(false, true);
			var delta = .00001;
			assert.closeTo(actual_stats.min, expected_stats.min, delta);
			assert.closeTo(actual_stats.max, expected_stats.max, delta);
			assert.closeTo(actual_stats.mean, expected_stats.mean, delta);
			assert.closeTo(actual_stats.std_dev, expected_stats.std_dev, delta);
		});
	});
});