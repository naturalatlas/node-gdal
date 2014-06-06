'use strict';

var ogr = require('../lib/gdal.js').ogr;
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('GeoJSON', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/park.geo.json");
			ds = ogr.open(filename);
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.getLayerCount(), 1);
		});

		describe('layer', function() {
			var layer;
			before(function() { layer = ds.getLayer(0); });
			it('should exist', function() {
				assert.ok(layer);
				assert.instanceOf(layer, ogr.Layer);
			});
			it('should have definition', function() {
				assert.ok(layer.getLayerDefn());
			})
		});
	});
});