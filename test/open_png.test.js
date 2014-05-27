'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;
var expect = require('chai').expect;

describe('Open', function() {
	describe('PNG', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/CM13ct.png");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.getRasterXSize(),1951);
			assert.equal(ds.getRasterYSize(),3109);
			assert.equal(ds.getRasterCount(),1);
		});

		it('should be able to read geotransform', function() {
			var expected_geotransform = [
				674425.2950174398,
				10,
				0,
				6462458.631486551,
				0,
				-10
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
				min: 1,
				max: 253,
				mean: 125.58997597595655,
				std_dev: 59.3013235226549
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
			assert.equal(size.x, 1951);
			assert.equal(size.y, 1);
		});

		it('should have file list', function() {
			var files = [
				path.join(__dirname,"data/CM13ct.png"),
				path.join(__dirname,"data/CM13ct.png.aux.xml"),
				path.join(__dirname,"data/CM13ct.pgw")
			];

			var actual_files = ds.getFileList();
			actual_files.sort();
			files.sort();

			expect(actual_files).to.eql(files);
		});
	});
});