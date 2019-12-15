var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal.CoordinateTransformation', function() {
	afterEach(gc);

	it('should be exposed', function() {
		assert.ok(gdal.CoordinateTransformation);
	});
	it('should handle Proj.4', function() {
		var srs0 = gdal.SpatialReference.fromProj4('+proj=lcc +lat_1=49 +lat_2=45 +lat_0=44.25 +lon_0=-109.5 +x_0=599999.9999976 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048 +no_defs');
		var srs1 = gdal.SpatialReference.fromProj4('+init=epsg:4326');

		var pt = new gdal.Point(1, 2);
		var ct = new gdal.CoordinateTransformation(srs0, srs1);
		pt.transform(ct);

		assert.closeTo(pt.x, -116.98546236548084, 0.1);
		assert.closeTo(pt.y, 43.99194884775841, 0.1);
	});
	it('should handle Proj.4 (EPSG)', function() {
		var srs0 = gdal.SpatialReference.fromProj4('+init=epsg:4326');
		var srs1 = gdal.SpatialReference.fromProj4('+init=epsg:32632');

		var pt = new gdal.Point(20, 30);
		var ct = new gdal.CoordinateTransformation(srs0, srs1);
		pt.transform(ct);

		assert.closeTo(pt.x, 1564201.4044502454, 0.1);
		assert.closeTo(pt.y, 3370263.469590679, 0.1);
	});
});
