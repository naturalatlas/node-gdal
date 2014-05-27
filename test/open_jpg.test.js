'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;
var expect = require('chai').expect;

describe('Open', function() {
	describe('JPG', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/a39se10.jpg");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.getRasterXSize(),1361);
			assert.equal(ds.getRasterYSize(),1744);
			assert.equal(ds.getRasterCount(),3);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [
				1485309.3937136543,
				3.28077925649814,
				0,
				603662.5109447651,
				0,
				-3.28077925649814
			];

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
			assert.equal(srs, '');

			var band = ds.getRasterBand(1);
			var expected_stats = {
				min: 0,
				max: 255,
				mean: 118.37471983296146,
				std_dev: 50.89389997162665
			};

			var actual_stats = band.getStatistics(0,1);
			assert.ok(Math.abs(expected_stats.min - actual_stats.min) < .00001);
			assert.ok(Math.abs(expected_stats.max - actual_stats.max) < .00001);
			assert.ok(Math.abs(expected_stats.mean - actual_stats.mean) < .00001);
			assert.ok(Math.abs(expected_stats.std_dev - actual_stats.std_dev) < .00001);
		});

		it('should be able to read block size', function() {
			var band = ds.getRasterBand(1);
			var size = band.getBlockSize();
			assert.equal(size.x, 1361);
			assert.equal(size.y, 1);
		});

		it('should have file list', function() {
			var files = [
				path.join(__dirname,"data/a39se10.jpg"),
				path.join(__dirname,"data/a39se10.jgw")
			];

			var actual_files = ds.getFileList();
			actual_files.sort();
			files.sort();

			expect(actual_files).to.eql(files);
		});
	});
});