var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal.LineString', function() {
	afterEach(gc);

	it('should be instantiable', function() {
		new gdal.LineString();
	});
	it('should inherit from Geometry', function() {
		assert.instanceOf(new gdal.LineString(), gdal.LineString);
		assert.instanceOf(new gdal.LineString(), gdal.Geometry);
	});
	describe('instance', function() {
		describe('getLength()', function() {
			it('should return length', function() {
				var line = new gdal.LineString();
				line.points.add(0, 0, 0);
				line.points.add(10, 10, 0);
				line.points.add(10, 20, 0);
				assert.closeTo(line.getLength(), Math.sqrt(10*10 + 10*10) + 10, 0.001);
			});
		});
		describe('"points" property', function() {
			describe('count()', function() {
				it('should return number of points', function() {
					var line = new gdal.LineString();
					line.points.add(0, 0, 0);
					line.points.add(10, 10, 0);
					line.points.add(10, 10, 2);
					assert.equal(line.points.count(), 3);
				});
			});
			describe('add()', function() {
				it('should accept gdal.Point instance', function() {
					var line = new gdal.LineString();
					var pt = new gdal.Point(2, 3);
					line.points.add(pt);
					var pt_result = line.points.get(0);
					assert.equal(pt_result.x, 2);
					assert.equal(pt_result.y, 3);
				});
				it('should accept gdal.Point array', function() {
					var line = new gdal.LineString();
					line.points.add([
						new gdal.Point(2, 3),
						new gdal.Point(4, 5)
					]);
					var pt_result1 = line.points.get(0);
					assert.equal(pt_result1.x, 2);
					assert.equal(pt_result1.y, 3);
					var pt_result2 = line.points.get(1);
					assert.equal(pt_result2.x, 4);
					assert.equal(pt_result2.y, 5);
				});
				it('should accept object', function() {
					var line = new gdal.LineString();
					line.points.add({x: 2, y: 3});
					var pt_result = line.points.get(0);
					assert.equal(pt_result.x, 2);
					assert.equal(pt_result.y, 3);
				});
				it('should accept object array', function() {
					var line = new gdal.LineString();
					line.points.add([
						{x: 2, y: 3},
						{x: 4, y: 5}
					]);
					var pt_result1 = line.points.get(0);
					assert.equal(pt_result1.x, 2);
					assert.equal(pt_result1.y, 3);
					var pt_result2 = line.points.get(1);
					assert.equal(pt_result2.x, 4);
					assert.equal(pt_result2.y, 5);
				});
				it('should accept x,y,z arguments', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					var pt_result = line.points.get(0);
					assert.equal(pt_result.x, 1);
					assert.equal(pt_result.y, 2);
					assert.equal(pt_result.z, 3);
				});
			});
			describe('set()', function() {
				it('should throw if point doesn\'t exist', function() {
					assert.throws(function() {
						var line = new gdal.LineString();
						line.points.set(1, new gdal.Point(1, 1));
						line.points.set(1, {x: 1, y: 1});
						line.points.set(1, 1, 1, 1);
					});
				});
				it('should accept object', function() {
					var line = new gdal.LineString();
					line.points.add(0, 0);
					line.points.set(0, {x: 1, y: 2});
					var pt = line.points.get(0);
					assert.equal(pt.x, 1);
					assert.equal(pt.y, 2);
				});
				it('should accept Point instance', function() {
					var line = new gdal.LineString();
					line.points.add(0, 0);
					line.points.set(0, new gdal.Point(1, 2));
					var pt = line.points.get(0);
					assert.equal(pt.x, 1);
					assert.equal(pt.y, 2);
				});
				it('should accept x,y,z arguments', function() {
					var line = new gdal.LineString();
					line.points.add(0, 0);
					line.points.set(0, 1, 2, 3);
					var pt = line.points.get(0);
					assert.equal(pt.x, 1);
					assert.equal(pt.y, 2);
					assert.equal(pt.z, 3);
				});
			});
			describe('get()', function() {
				it('should return null if point doesn\'t exist', function() {
					var line = new gdal.LineString();
					assert.isNull(line.points.get(2));
				});
				it('should Point instance', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					var pt = line.points.get(0);
					assert.instanceOf(pt, gdal.Point);
					assert.equal(pt.x, 1);
					assert.equal(pt.y, 2);
					assert.equal(pt.z, 3);
				});
			});
			describe('resize()', function() {
				it('should adjust the number of points', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					line.points.add(2, 3, 4);
					line.points.add(3, 4, 5);
					line.points.resize(2);
					assert.equal(line.points.count(), 2);
				});
			});
			describe('reverse()', function() {
				it('should flip order of points', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					line.points.add(2, 3, 4);
					line.points.add(3, 4, 5);
					line.points.reverse();
					var p1 = line.points.get(0);
					var p2 = line.points.get(1);
					var p3 = line.points.get(2);
					assert.equal(p1.x, 3);
					assert.equal(p2.x, 2);
					assert.equal(p3.x, 1);
				});
			});
			describe('forEach()', function() {
				it('should stop if callback returns false', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					line.points.add(2, 3, 4);
					line.points.add(3, 4, 5);
					var i = 0, count = 0;
					line.points.forEach(function(pt) {
						count++;
						if (++i === 2) return false;
					});
					assert.equal(count, 2);
				});
				it('should iterate through all points', function() {
					var x_expected = [1, 2, 3];
					var x_actual = [];
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					line.points.add(2, 3, 4);
					line.points.add(3, 4, 5);
					line.points.forEach(function(pt) {
						x_actual.push(pt.x);
					});
					assert.deepEqual(x_actual, x_actual);
				});
			});
			describe('toArray()', function() {
				it('should return array of Point instances', function() {
					var line = new gdal.LineString();
					line.points.add(1, 2, 3);
					line.points.add(2, 3, 4);
					line.points.add(3, 4, 5);
					var points = line.points.toArray();
					assert.lengthOf(points, 3);
					assert.instanceOf(points[0], gdal.Point);
					assert.instanceOf(points[1], gdal.Point);
					assert.instanceOf(points[2], gdal.Point);
					assert.equal(points[0].x, 1);
					assert.equal(points[1].x, 2);
					assert.equal(points[2].x, 3);
				});
			});
		});
	});
});