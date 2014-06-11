'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('PNG', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/CM13ct.png");
			ds = gdal.open(filename);
		});

		it('should be able to read raster size', function() {
			assert.equal(ds.size.x,1951);
			assert.equal(ds.size.y,3109);
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
			var delta = .00001;
			assert.closeTo(actual_geotransform[0], expected_geotransform[0], delta);
			assert.closeTo(actual_geotransform[1], expected_geotransform[1], delta);
			assert.closeTo(actual_geotransform[2], expected_geotransform[2], delta);
			assert.closeTo(actual_geotransform[3], expected_geotransform[3], delta);
			assert.closeTo(actual_geotransform[4], expected_geotransform[4], delta);
			assert.closeTo(actual_geotransform[5], expected_geotransform[5], delta);
		});

		it('should not have projection', function() {
			assert.isNull(ds.srs);
		});

		it('should be able to read statistics', function() {
			var band = ds.getRasterBand(1);
			var expected_stats = {
				min: 1,
				max: 253,
				mean: 125.58997597595655,
				std_dev: 59.3013235226549
			};

			var actual_stats = band.getStatistics(false, true);
			var delta = .00001;
			assert.closeTo(actual_stats.min, expected_stats.min, delta);
			assert.closeTo(actual_stats.max, expected_stats.max, delta);
			assert.closeTo(actual_stats.mean, expected_stats.mean, delta);
			assert.closeTo(actual_stats.std_dev, expected_stats.std_dev, delta);
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

			assert.deepEqual(actual_files, files);
		});
	});
});