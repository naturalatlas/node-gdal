'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	describe('ESRI Shapefile', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname,"data/sample.shp");
			ds = gdal.open(filename);
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.layers.count(), 1);
		});

		describe('layer', function() {
			var layer;
			before(function() { layer = ds.layers.get(0); });
			it('should have all fields defined', function() {
				assert.equal(layer.fields.count(), 8);
				assert.deepEqual(layer.fields.getNames(), [
					'path',
					'name',
					'type',
					'long_name',
					'fips_num',
					'fips',
					'state_fips',
					'state_abbr'
				]);
			});

			describe('features', function() {
				it('should be readable', function() {
					assert.equal(layer.features.count(), 23);
					var feature = layer.features.get(0);
					var fields = feature.fields.toJSON();

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