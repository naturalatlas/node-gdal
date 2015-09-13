'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe.skip('gdal.Envelope', function() {
	afterEach(gc);

	it('should be instantiable', function() {
		new gdal.Envelope();
	});
	describe('instance', function() {
		it('should have "minX" property', function() {
			var envelope = new gdal.Envelope({minX: 5, maxX: 0, minY: 0, maxY: 0});
			assert.equal(envelope.minX, 5);
		});
		it('should have "maxX" property', function() {
			var envelope = new gdal.Envelope({minX: 0, maxX: 5, minY: 0, maxY: 0});
			assert.equal(envelope.maxX, 5);
		});
		it('should have "minY" property', function() {
			var envelope = new gdal.Envelope({minX: 0, maxX: 0, minY: 5, maxY: 0});
			assert.equal(envelope.minY, 5);
		});
		it('should have "maxY" property', function() {
			var envelope = new gdal.Envelope({minX: 0, maxX: 0, minY: 0, maxY: 5});
			assert.equal(envelope.maxY, 5);
		});
		describe('isEmpty()', function() {
			it('should return true when all components zero', function() {
				var envelope = new gdal.Envelope();
				assert.isTrue(envelope.isEmpty());
			});
			it('should return false when a component is non-zero', function() {
				var envelope = new gdal.Envelope({minX: 0, maxX: 5, minY: 0, maxY: 0});
				assert.isFalse(envelope.isEmpty());
			});
		});
		describe('merge()', function() {
			describe('w/x,y arguments', function() {
				it('should expand envelope', function() {
					var envelope = new gdal.Envelope({minX: -1, maxX: 1, minY: -2, maxY: 2});
					envelope.merge(2, 3);
					assert.equal(envelope.minX, -1);
					assert.equal(envelope.minY, -2);
					assert.equal(envelope.maxX, 2);
					assert.equal(envelope.maxY, 3);
				});
			});
			describe('w/envelope argument', function() {
				it('should expand envelope', function() {
					var envelopeA = new gdal.Envelope({minX: -1, maxX: 1, minY: -2, maxY: 2});
					var envelopeB = new gdal.Envelope({minX: -2, maxX: 10, minY: -1, maxY: 1});
					envelopeA.merge(envelopeB);
					assert.equal(envelopeA.minX, -2);
					assert.equal(envelopeA.minY, -2);
					assert.equal(envelopeA.maxX, 10);
					assert.equal(envelopeA.maxY, 2);
				});
			});
		});
		describe('intersects()', function() {
			it('should determine if envelopes touch', function() {
				var envelopeA = new gdal.Envelope({minX: 1, maxX: 2, minY: 1, maxY: 2});
				var envelopeB = new gdal.Envelope({minX: 2, maxX: 4, minY: 1, maxY: 2});
				var envelopeC = new gdal.Envelope({minX: 10, maxX: 20, minY: 10, maxY: 20});
				assert.isTrue(envelopeA.intersects(envelopeB));
				assert.isFalse(envelopeA.intersects(envelopeC));
			});
		});
		describe('contains()', function() {
			it('should determine if it fully contains the other envelope', function() {
				var envelopeA = new gdal.Envelope({minX: -10, maxX: 10, minY: -10, maxY: 10});
				var envelopeB = new gdal.Envelope({minX: -1, maxX: 1, minY: -1, maxY: 1});
				var envelopeC = new gdal.Envelope({minX: -1, maxX: 1, minY: -1, maxY: 20});
				assert.isTrue(envelopeA.contains(envelopeB));
				assert.isFalse(envelopeA.contains(envelopeC));
			});
		});
		describe('intersect()', function() {
			it('should yield empty envelope if the two don\'t intersect', function() {
				var envelopeA = new gdal.Envelope({minX: 1, maxX: 2, minY: 1, maxY: 2});
				var envelopeC = new gdal.Envelope({minX: 10, maxX: 20, minY: 10, maxY: 20});
				envelopeA.intersect(envelopeC);
				assert.equal(envelopeA.minX, 0);
				assert.equal(envelopeA.maxX, 0);
				assert.equal(envelopeA.minY, 0);
				assert.equal(envelopeA.maxY, 0);
			});
			it('should yield other envelope if current envelope is empty and other envelope intersects 0,0', function() {
				var envelopeA = new gdal.Envelope();
				var envelopeC = new gdal.Envelope({minX: -10, maxX: 20, minY: -10, maxY: 20});
				envelopeA.intersect(envelopeC);
				assert.equal(envelopeA.minX, -10);
				assert.equal(envelopeA.maxX, 20);
				assert.equal(envelopeA.minY, -10);
				assert.equal(envelopeA.maxY, 20);
			});
			it('should otherwise yield intersection', function() {
				var envelopeA = new gdal.Envelope({minX: -10, maxX: 10, minY: -10, maxY: 10});
				var envelopeB = new gdal.Envelope({minX: -2, maxX: 12, minY: -1, maxY: 1});
				envelopeA.intersect(envelopeB);
				assert.equal(envelopeA.minX, -2);
				assert.equal(envelopeA.maxX, 10);
				assert.equal(envelopeA.minY, -1);
				assert.equal(envelopeA.maxY, 1);
			});
		});
		describe('toPolygon()', function() {
			it('should return Polygon', function() {
				var envelope = new gdal.Envelope({minX: -1, maxX: 1, minY: -2, maxY: 2});
				var polygon  = envelope.toPolygon();
				envelope = polygon.getEnvelope();
				assert.instanceOf(polygon, gdal.Polygon);
				assert.equal(envelope.minX, -1);
				assert.equal(envelope.maxX, 1);
				assert.equal(envelope.minY, -2);
				assert.equal(envelope.maxY, 2);
			});
		});
	});
});