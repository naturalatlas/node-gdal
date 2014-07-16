'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal', function() {
	describe('"version" property', function() {
		it('should exist', function() {
			assert.match(gdal.version, /^\d+\.\d+\.\d+$/);
		});
	});
	describe('"config" property', function() {
		describe('get()', function() {
			it('should not throw', function() {
				gdal.config.get('CPL_LOG');
			});
		});
		describe('set()', function() {
			it('should set option', function() {
				gdal.config.set('CPL_DEBUG', 'ON');
				assert.equal(gdal.config.get('CPL_DEBUG'), 'ON');
				gdal.config.set('CPL_DEBUG', null);
				assert.isNull(gdal.config.get('CPL_DEBUG'));
			});
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