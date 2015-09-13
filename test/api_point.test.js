var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe.skip('gdal.Point', function() {
	afterEach(gc);

	var point2d;
	var point3d;
	before(function() {
		point2d = new gdal.Point(1,2);
		point3d = new gdal.Point(1,2,3);
	})
	it('should inherit from Geometry', function() {
		assert.instanceOf(point2d, gdal.Point);
		assert.instanceOf(point2d, gdal.Geometry);
		assert.instanceOf(point3d, gdal.Point);
		assert.instanceOf(point3d, gdal.Geometry);
	});
	it('should be valid', function() {
		assert.equal(point2d.isValid(), true);
		assert.equal(point3d.isValid(), true);
	});
	it('should be simple', function() {
		assert.equal(point2d.isSimple(), true);
		assert.equal(point3d.isSimple(), true);
	});
	describe('instance', function() {
		describe('"x","y","z" properties', function() {
			it('should be gettable', function() {
				assert.equal(point2d.x, 1);
				assert.equal(point2d.y, 2);
				assert.equal(point2d.z, 0);
				assert.equal(point3d.x, 1);
				assert.equal(point3d.y, 2);
				assert.equal(point3d.z, 3);
			});
			it('should be settable', function() {
				var pt = new gdal.Point(1,2,3);
				pt.x = 4;
				pt.y = 5;
				pt.z = 6;
				assert.equal(pt.x, 4);
				assert.equal(pt.y, 5);
				assert.equal(pt.z, 6);
			});
		});
		describe('swapXY()', function() {
			it('should flip x,y coordinates', function() {
				var pt = new gdal.Point(1,2,3);
				pt.swapXY();
				assert.equal(pt.x, 2);
				assert.equal(pt.y, 1);
				assert.equal(pt.z, 3);
			});
		});
	});
});