'use strict';

var ogr = require('../lib/gdal.js').ogr;
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('ESRI Shapefile', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/sample.shp");
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
					assert.equal(defn.getFieldCount(), 8);
					assert.equal(defn.getFieldDefn(0).getName(), 'path');
					assert.equal(defn.getFieldDefn(1).getName(), 'name');
					assert.equal(defn.getFieldDefn(2).getName(), 'type');
					assert.equal(defn.getFieldDefn(3).getName(), 'long_name');
					assert.equal(defn.getFieldDefn(4).getName(), 'fips_num');
					assert.equal(defn.getFieldDefn(5).getName(), 'fips');
					assert.equal(defn.getFieldDefn(6).getName(), 'state_fips');
					assert.equal(defn.getFieldDefn(7).getName(), 'state_abbr');
				});
			});
			describe('features', function() {
				it('should be readable', function() {
					assert.equal(layer.getFeatureCount(), 23);
					var feature = layer.getFeature(0);
					var fields = feature.getFields();

					assert.deepEqual(fields, {
						'fips': 'US56029',
						'fips_num': '56029',
						'long_name': 'Park County',
						'name': 'Park',
						'path': 'US.WY.PARK',
						'state_abbr': 'WY',
						'state_fips': '56',
						'type': 'County'
					});
				});
			});
		});
	});
});