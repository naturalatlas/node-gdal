'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('assert');

describe('Open', function() {
	describe('HFA (IMG)', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/dem_azimuth50_pa.img");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.getRasterXSize(),495);
			assert.equal(ds.getRasterYSize(),286);
			assert.equal(ds.getRasterCount(),1);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [-215000, 1000, 0, 365000, 0, -1000];

			var actual_geotransform = ds.getGeoTransform();
			assert.ok(Math.abs(actual_geotransform[0] - expected_geotransform[0]) < .00001);
			assert.ok(Math.abs(actual_geotransform[1] - expected_geotransform[1]) < .00001);
			assert.ok(Math.abs(actual_geotransform[2] - expected_geotransform[2]) < .00001);
			assert.ok(Math.abs(actual_geotransform[3] - expected_geotransform[3]) < .00001);
			assert.ok(Math.abs(actual_geotransform[4] - expected_geotransform[4]) < .00001);
			assert.ok(Math.abs(actual_geotransform[5] - expected_geotransform[5]) < .00001);
		});

		it('should be able to read statistics', function() {
			var srs = ds.getProjectionRef();
			assert.ok(srs.indexOf('PROJCS') > -1);

			var band = ds.getRasterBand(1);
			var expected_stats = {
				min: 177,
				max: 182,
				mean: 179.90853881836,
				std_dev: 0.37681820988655
			};

			var actual_stats = band.getStatistics(0,1);
			assert.ok(Math.abs(expected_stats.min - actual_stats.min) < .00001);
			assert.ok(Math.abs(expected_stats.max - actual_stats.max) < .00001);
			assert.ok(Math.abs(expected_stats.mean - actual_stats.mean) < .00001);
			assert.ok(Math.abs(expected_stats.std_dev - actual_stats.std_dev) < .00001);
		});
	});
});