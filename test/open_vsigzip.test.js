var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	afterEach(gc);

	describe('vsigzip', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname, 'data/vsigzip/hp40ne.gz');
			ds = gdal.open(filename);
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.layers.count(), 4);
		});

		describe('layer', function() {
			var layer;
			before(function() { layer = ds.layers.get(0); });
			it('should have all fields defined', function() {
				assert.equal(layer.fields.count(), 8);
				assert.deepEqual(layer.fields.getNames(), [
					'fid',
					'featureCode',
					'featureDescription',
					'anchorPosition',
					'font',
					'height',
					'orientation',
					'textString'
				]);
			});
			var layer2;
			before(function() { layer2 = ds.layers.get(1); });
			describe('field properties', function() {
				it('should evaluate datatypes', function() {
					assert.equal('string', layer2.fields.get(0).type);
					assert.equal('integer', layer2.fields.get(1).type);
					assert.equal('string', layer2.fields.get(2).type);
				});
			});
			describe('features', function() {
				it('should be readable', function() {
					assert.equal(layer2.features.count(), 381);
					// layer2.features.get(0); doesn't work, as there is no 'id', but 'fid' instead
					var feature = layer2.features.next();
					var fields = feature.fields.toObject();

					assert.deepEqual(fields, {
						'fid': 'ID_105',
						'featureCode': 15600,
						'featureDescription': 'Water Feature'
					});
				});
			});
		});
	});
});
