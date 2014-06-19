'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('Driver', function() {
	var drivers = [
		"GEOJSON", "VRT:vector", "ESRI Shapefile"
	];

	drivers.forEach(function(o) {
		describe(o + ' (OGR)', function() {
			it('should exist', function() {
				assert.ok(gdal.drivers.get(o));
			});
		});
	});
});
