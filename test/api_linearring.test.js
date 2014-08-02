'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('gdal.LinearRing', function() {
	afterEach(gc);

	it('should be instantiable', function() {
		new gdal.LinearRing();
	});
	it('should inherit from LineString', function() {
		assert.instanceOf(new gdal.LinearRing(), gdal.LinearRing);
		assert.instanceOf(new gdal.LinearRing(), gdal.LineString);
		assert.instanceOf(new gdal.LinearRing(), gdal.Geometry);
	});
	describe('instance', function() {
		describe('getArea()', function() {
			it('should return area', function() {
				var ring = new gdal.LinearRing();
				ring.points.add(0, 0, 0);
				ring.points.add(10, 0, 0);
				ring.points.add(10, 10, 0);
				ring.points.add(0, 10, 0);
				ring.points.add(0, 0, 0);
				assert.closeTo(ring.getArea(), 100, 0.001);
			});
		});
	});
});