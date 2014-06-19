var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;
var WGS84 = 'GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]]';

describe('Geometry', function() {
	describe('toJSON()', function() {
		it('should return valid result', function() {
			var point2d = new gdal.Point(1,2);
			assert.deepEqual(JSON.parse(point2d.toJSON()), {
				type: 'Point',
				coordinates: [1, 2]
			});
			var point3d = new gdal.Point(1,2,3);
			assert.deepEqual(JSON.parse(point3d.toJSON()), {
				type: 'Point',
				coordinates: [1, 2, 3]
			});
		});
	});
	describe('toKML()', function() {
		it('should return valid result', function() {
			var point2d = new gdal.Point(1,2);
			assert.equal(point2d.toKML(), '<Point><coordinates>1,2</coordinates></Point>');
			var point3d = new gdal.Point(1,2,3);
			assert.equal(point3d.toKML(), '<Point><coordinates>1,2,3</coordinates></Point>');
		});
	});
	describe('toWKT()', function() {
		it('should return valid result', function() {
			var point2d = new gdal.Point(1,2);
			assert.equal(point2d.toWKT(), 'POINT (1 2)');
			var point3d = new gdal.Point(1,2,3);
			assert.equal(point3d.toWKT(), 'POINT (1 2 3)');
		});
	});
	describe('toGML()', function() {
		it('should return valid result', function() {
			var point2d = new gdal.Point(1,2);
			assert.equal(point2d.toGML(), '<gml:Point><gml:coordinates>1,2</gml:coordinates></gml:Point>');
			var point3d = new gdal.Point(1,2,3);
			assert.equal(point3d.toGML(), '<gml:Point><gml:coordinates>1,2,3</gml:coordinates></gml:Point>');
		});
	});

	describe('"srs" property', function() {
		it('should be able to be get', function() {
			var point = new gdal.Point(0,0);
			assert.equal(point.srs, null);

			point.srs = gdal.SpatialReference.fromWKT(WGS84);
			assert.instanceOf(point.srs, gdal.SpatialReference);
			assert.equal(point.srs.toWKT(), WGS84);
		});
		it('should be able to be set', function() {
			var point = new gdal.Point(1,2);
			point.srs = gdal.SpatialReference.fromWKT(WGS84);
			point.srs = null;
		});
		it('must require SpatialReference when setting', function() {
			var point = new gdal.Point(1,2);
			assert.throws(function() {
				point.srs = 'invalid';
			});
		});
	});

	describe('comparison function', function() {
		var ring, square, point_inner, point_inner_clone, point_outer;
		before(function() {
			ring = new gdal.LinearRing();
			ring.points.add({x: 0, y: 0});
			ring.points.add({x: 10, y: 0});
			ring.points.add({x: 10, y: 10});
			ring.points.add({x: 0, y: 10});
			ring.closeRings();

			square = new gdal.Polygon();
			square.rings.add(ring);

			point_inner = new gdal.Point(5, 5);
			point_outer = new gdal.Point(0, 20);
			point_inner_clone = new gdal.Point(5, 5);
		});
		describe('contains()', function() {
			it('should return correct result', function() {
				assert.equal(square.contains(point_inner), true);
				assert.equal(square.contains(point_outer), false);
			});
		});
		describe('within()', function() {
			it('should return correct result', function() {
				assert.equal(point_inner.within(square), true);
				assert.equal(point_outer.within(square), false);
			});
		});
		describe('intersects()', function() {
			it('should return correct result', function() {
				assert.equal(point_inner.intersects(square), true);
				assert.equal(point_outer.intersects(square), false);
			});
		});
		describe('equals()', function() {
			it('should return correct result', function() {
				assert.equal(point_inner.equals(square), false);
				assert.equal(point_outer.equals(square), false);
				assert.equal(point_inner.equals(point_inner_clone), true);
			});
		});
		describe('disjoint()', function() {
			it('should return correct result', function() {
				assert.equal(point_inner.disjoint(square), false);
				assert.equal(point_outer.disjoint(square), true);
			});
		});
		describe('crosses()', function() {
			it('should return correct result', function() {
				var line_cross = new gdal.LineString();
				line_cross.points.add(-1, -1);
				line_cross.points.add(11, 11);
				var line_nocross = new gdal.LineString();
				line_nocross.points.add(-1, -1);
				line_nocross.points.add(-1, 11);

				assert.equal(point_inner.crosses(square), false);
				assert.equal(point_outer.crosses(square), false);
				assert.equal(line_cross.crosses(square), true);
				assert.equal(line_nocross.crosses(square), false);
			});
		});
		describe('overlaps()', function() {
			it('should return correct result', function() {
				var ring1 = new gdal.LinearRing();
				ring1.points.add({x: 1, y: 0});
				ring1.points.add({x: 11, y: 0});
				ring1.points.add({x: 11, y: 10});
				ring1.points.add({x: 1, y: 10});
				ring1.closeRings();

				var square1 = new gdal.Polygon();
				square1.rings.add(ring1);

				var ring2 = new gdal.LinearRing();
				ring2.points.add({x: 100, y: 0});
				ring2.points.add({x: 110, y: 0});
				ring2.points.add({x: 110, y: 10});
				ring2.points.add({x: 100, y: 10});
				ring2.closeRings();

				var square2 = new gdal.Polygon();
				square2.rings.add(ring2);

				assert.equal(square1.overlaps(square), true);
				assert.equal(square2.overlaps(square), false);
			});
		});
		describe('touches()', function() {
			it('should return correct result', function() {
				var point_edge = new gdal.Point(10, 0);
				assert.equal(point_edge.touches(square), true);
				assert.equal(point_outer.touches(square), false);
			});
		});
	});
});