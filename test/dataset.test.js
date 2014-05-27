'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('assert');

describe('Dataset', function() {
	it('should be exposed', function() {
		assert.ok(gdal.Dataset);
	});

	describe('getRasterBand()', function() {
		var ds;
		before(function() {
			ds = gdal.open(__dirname + "/data/sample.tif");
		});

		it('should throw when invalid band', function() {
			assert.throws(function() {
				ds.getRasterBand(2);
			}, /band not found/);
		});
		it('should return RasterBand instance when valid band', function() {
			var band = ds.getRasterBand(1);
			assert.ok(band instanceof gdal.RasterBand);
		});
	});

});