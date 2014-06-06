'use strict';

var assert = require('chai').assert;
var ogr = require('../lib/gdal.js').ogr;

describe('Driver', function() {
	var drivers = [
		"GEOJSON", "VRT", "ESRI Shapefile"
	];

	drivers.forEach(function(o) {
		describe(o + ' (OGR)', function() {
			it('should exist', function() {
				assert.ok(ogr.getDriverByName(o));
			});
		});
	});
});
