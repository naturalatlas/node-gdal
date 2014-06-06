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
			describe('definition', function() {
				var defn;
				before(function() {
					defn = layer.getLayerDefn();
				});
				it('should exist', function() {
					assert.ok(defn);
					assert.instanceOf(defn, ogr.FeatureDefn);
				});
				it('should have all fields defined', function() {
					assert.equal(defn.getFieldCount(), 3);
					assert.equal(defn.getFieldDefn(0).getName(), 'kind');
					assert.equal(defn.getFieldDefn(1).getName(), 'name');
					assert.equal(defn.getFieldDefn(2).getName(), 'state');
				});
			});
			describe('features', function() {
				it('should have all fields', function() {
					assert.equal(layer.getFeatureCount(), 1);
					var feature = layer.getFeature(0);
					var fields = feature.getFields();

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