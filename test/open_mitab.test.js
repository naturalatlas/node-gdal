var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Open', function() {
	afterEach(gc);

	describe('MapInfo', function() {
		var filename, ds;

		it('should not throw', function() {
			filename = path.join(__dirname, 'data/mitab/EXTRACT_POLYGON.mif');
			ds = gdal.open(filename);
		});
		it('should be able to read layer count', function() {
			assert.equal(ds.layers.count(), 1);
		});
		it('should be able to read features', function() {
			var layer = ds.layers.get(0);
			var featureCount = layer.features.count();
			assert.equal(featureCount, 1, 'layer.features.count()');
			assert.instanceOf(layer.features.first().getGeometry(), gdal.Polygon);
		});
	});
});
