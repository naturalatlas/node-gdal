'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('HFA (IMG)', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/dem_azimuth50_pa.img");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.size.x,495);
			assert.equal(ds.size.y,286);
			assert.equal(ds.getRasterCount(),1);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [-215000, 1000, 0, 365000, 0, -1000];

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
			assert.match(ds.srs.toString(), /PROJCS/);

			var band = ds.getRasterBand(1);
			var expected_stats = {
				min: 177,
				max: 182,
				mean: 179.90853881836,
				std_dev: 0.37681820988655
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