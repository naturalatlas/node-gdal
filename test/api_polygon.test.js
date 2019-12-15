var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('gdal.Polygon', function() {
	afterEach(gc);

	it('should be instantiable', function() {
		new gdal.Polygon();
	});
	it('should inherit from Geometry', function() {
		assert.instanceOf(new gdal.Polygon(), gdal.Polygon);
		assert.instanceOf(new gdal.Polygon(), gdal.Geometry);
	});
	describe('instance', function() {
		describe('"rings" property', function() {
			describe('get()', function() {
				it('should return null if ring doesn\'t exist', function() {
					var polygon = new gdal.Polygon();
					assert.isNull(polygon.rings.get(2));
				});
				it('should return LinearRing instance', function() {
					var polygon = new gdal.Polygon();
					var ring = new gdal.LinearRing();
					ring.points.add(0, 0, 0);
					ring.points.add(10, 0, 0);
					ring.points.add(10, 10, 0);
					ring.points.add(0, 10, 0);
					ring.points.add(0, 0, 0);
					polygon.rings.add(ring);
					assert.instanceOf(polygon.rings.get(0), gdal.LinearRing);
				});
			});
			describe('count()', function() {
				it('should return ring count', function() {
					var polygon = new gdal.Polygon();
					assert.equal(polygon.rings.count(), 0);

					var ring = new gdal.LinearRing();
					ring.points.add(0, 0, 0);
					ring.points.add(10, 0, 0);
					ring.points.add(10, 10, 0);
					ring.points.add(0, 10, 0);
					ring.points.add(0, 0, 0);
					polygon.rings.add(ring);
					assert.equal(polygon.rings.count(), 1);
				});
			});
			describe('add()', function() {
				it('should add a ring', function() {
					var polygon = new gdal.Polygon();
					var ring = new gdal.LinearRing();
					ring.points.add(0, 0, 0);
					ring.points.add(10, 0, 0);
					ring.points.add(10, 11, 0);
					ring.points.add(0, 10, 0);
					ring.points.add(0, 0, 0);
					polygon.rings.add(ring);
					var ring_result = polygon.rings.get(0);
					assert.instanceOf(ring_result, gdal.LinearRing);
					assert.equal(ring_result.points.get(2).y, 11);
				});
				it('should accept multiple LinearRing instances', function() {
					var ring1 = new gdal.LinearRing();
					ring1.points.add(0, 0, 0);
					ring1.points.add(10, 0, 0);
					ring1.points.add(10, 11, 0);
					ring1.points.add(0, 10, 0);
					ring1.points.add(0, 0, 0);
					var ring2 = new gdal.LinearRing();
					ring2.points.add(1, 0, 0);
					ring2.points.add(11, 0, 0);
					ring2.points.add(11, 11, 0);
					ring2.points.add(1, 10, 0);
					ring2.points.add(1, 0, 0);

					var polygon = new gdal.Polygon();
					polygon.rings.add([ring1, ring2]);
					assert.equal(polygon.rings.count(), 2);
				});
			});
			describe('forEach()', function() {
				var polygon;
				before(function() {
					var ring1 = new gdal.LinearRing();
					ring1.points.add(0, 0, 0);
					ring1.points.add(10, 0, 0);
					ring1.points.add(10, 11, 0);
					ring1.points.add(0, 10, 0);
					ring1.points.add(0, 0, 0);
					var ring2 = new gdal.LinearRing();
					ring2.points.add(1, 0, 0);
					ring2.points.add(11, 0, 0);
					ring2.points.add(11, 11, 0);
					ring2.points.add(1, 10, 0);
					ring2.points.add(1, 0, 0);
					var ring3 = new gdal.LinearRing();
					ring3.points.add(2, 0, 0);
					ring3.points.add(20, 0, 0);
					ring3.points.add(20, 11, 0);
					ring3.points.add(3, 10, 0);
					ring3.points.add(3, 0, 0);

					polygon = new gdal.Polygon();
					polygon.rings.add([ring1, ring2, ring3]);
				});
				it('should stop if callback returns false', function() {
					var count = 0;
					polygon.rings.forEach(function(pt, i) {
						count++;
						assert.isNumber(i);
						if (i === 1) return false;
					});
					assert.equal(count, 2);
				});
				it('should iterate through all points', function() {
					var result = [];
					polygon.rings.forEach(function(ring) {
						result.push(ring.points.toArray().map(function(pt) {
							return pt.toJSON();
						}));
					});

					assert.deepEqual(result, [
						['{ "type": "Point", "coordinates": [ 0.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 10.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 10.0, 11.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 0.0, 10.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 0.0, 0.0, 0.0 ] }' ],
						[ '{ "type": "Point", "coordinates": [ 1.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 11.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 11.0, 11.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 1.0, 10.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 1.0, 0.0, 0.0 ] }' ],
						[ '{ "type": "Point", "coordinates": [ 2.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 20.0, 0.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 20.0, 11.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 3.0, 10.0, 0.0 ] }',
							'{ "type": "Point", "coordinates": [ 3.0, 0.0, 0.0 ] }' ]
    																																																																																																																																																																				]);
				});
			});
			describe('map()', function() {
				it('should operate normally', function() {
					var polygon = new gdal.Polygon();
					var ring = new gdal.LinearRing();
					ring.points.add(0, 0, 0);
					ring.points.add(10, 0, 0);
					ring.points.add(10, 10, 0);
					ring.points.add(0, 11, 0);
					ring.points.add(0, 0, 0);
					polygon.rings.add(ring);

					var result = polygon.rings.map(function(ring, i) {
						assert.isNumber(i);
						assert.instanceOf(ring, gdal.LinearRing);
						return 'a';
					});

					assert.isArray(result);
					assert.lengthOf(result, 1);
					assert.equal(result[0], 'a');
				});
			});
			describe('toArray()', function() {
				it('should return array of LinearRing instances', function() {
					var polygon = new gdal.Polygon();
					var ring = new gdal.LinearRing();
					ring.points.add(0, 0, 0);
					ring.points.add(10, 0, 0);
					ring.points.add(10, 10, 0);
					ring.points.add(0, 11, 0);
					ring.points.add(0, 0, 0);
					polygon.rings.add(ring);
					var array = polygon.rings.toArray();
					assert.isArray(array);
					assert.lengthOf(array, 1);
					assert.instanceOf(array[0], gdal.LinearRing);
					assert.equal(array[0].points.get(3).y, 11);
				});
			});
		});
		describe('getArea()', function() {
			it('should return area', function() {
				var polygon = new gdal.Polygon();
				var ring = new gdal.LinearRing();
				ring.points.add(0, 0, 0);
				ring.points.add(10, 0, 0);
				ring.points.add(10, 10, 0);
				ring.points.add(0, 10, 0);
				ring.points.add(0, 0, 0);
				polygon.rings.add(ring);
				assert.closeTo(ring.getArea(), 100, 0.001);
			});
		});
	});
});
