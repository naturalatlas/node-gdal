'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	afterEach(gc);

	describe('GPX', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/gpx/spiritmountain.gpx");
			ds = gdal.open(filename, 'r', 'GPX');
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.layers.count(), 5);
		});

		describe('layer', function() {
			var layer;
			it('should exist', function() {
				layer = ds.layers.get(0);
				assert.ok(layer);
				assert.instanceOf(layer, gdal.Layer);
			});
			it('should have all fields defined', function() {
				var names = layer.fields.getNames();
				assert.include(names, 'ele');
				assert.include(names, 'time');
				assert.include(names, 'name');
				assert.include(names, 'cmt');
				assert.include(names, 'sym');
			});
			describe('features', function() {
				it('should be readable', function() {
					assert.equal(layer.features.count(), 2);
					var feature = layer.features.get(0);
					var fields = feature.fields.toJSON();

					assert.closeTo(fields.ele, 1975.311646, 0.000001);
					assert.deepEqual(fields.time, {
						year: 2014,
						month: 5,
						day: 27,
						hour: 17,
						minute: 25,
						second: 13,
						timezone: 100
					});
					assert.equal(fields.name, '005');
					assert.equal(fields.cmt, 'PARK');
					assert.equal(fields.sym, 'Flag, Blue');
				});
			});
		});
	});
});