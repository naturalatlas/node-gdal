'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	afterEach(gc);

	describe('GeoJSON', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/park.geo.json");
			ds = gdal.open(filename);
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.layers.count(), 1);
		});

		describe('layer', function() {
			var layer;
			it('should exist', function() {
				layer = ds.layers.get(0);
				assert.ok(layer);
				assert.instanceOf(layer, gdal.Layer);
			});
			it('should have all fields defined', function() {
				assert.equal(layer.fields.count(), 3);
				assert.deepEqual(layer.fields.getNames(), [
					'kind',
					'name',
					'state'
				]);
			});
			describe('features', function() {
				it('should be readable', function() {
					assert.equal(layer.features.count(), 1);
					var feature = layer.features.get(0);
					var fields = feature.fields.toJSON();

					assert.deepEqual(fields, {
						'kind': 'county',
						'state': 'WY',
						'name': 'Park'
					});
				});
			});
		});
	});
});