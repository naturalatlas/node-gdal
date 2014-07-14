'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal', function() {
	describe('"version" property', function() {
		it('should exist', function() {
			assert.match(gdal.version, /^\d+\.\d+\.\d+$/);
		});
	});
	describe('decToDMS()', function() {
		it('should throw when axis not provided', function() {
			assert.throws(function() {
				gdal.decToDMS(12.2);
			});
		});
		it('should return correct result', function() {
			assert.equal(gdal.decToDMS(14.12511, 'lat', 2), ' 14d 7\'30.40"N');
			assert.equal(gdal.decToDMS(14.12511, 'lat', 1), ' 14d 7\'30.4"N');
			assert.equal(gdal.decToDMS(14.12511, 'long', 2), ' 14d 7\'30.40"E');
			assert.equal(gdal.decToDMS(14.12511, 'long', 1), ' 14d 7\'30.4"E');
		});
	});
});