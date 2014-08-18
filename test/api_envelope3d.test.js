'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('gdal.Envelope3D', function() {
	afterEach(gc);

	it('should be instantiable', function() {
		new gdal.Envelope3D();
	});
	describe('instance', function() {
		it('should have "minX" property', function() {
			var envelope = new gdal.Envelope3D({minX: 5, maxX: 0, minY: 0, maxY: 0, minZ: 0, maxZ: 0});
			assert.equal(envelope.minX, 5);
		});
		it('should have "maxX" property', function() {
			var envelope = new gdal.Envelope3D({minX: 0, maxX: 5, minY: 0, maxY: 0, minZ: 0, maxZ: 0});
			assert.equal(envelope.maxX, 5);
		});
		it('should have "minY" property', function() {
			var envelope = new gdal.Envelope3D({minX: 0, maxX: 0, minY: 5, maxY: 0, minZ: 0, maxZ: 0});
			assert.equal(envelope.minY, 5);
		});
		it('should have "maxY" property', function() {
			var envelope = new gdal.Envelope3D({minX: 0, maxX: 0, minY: 0, maxY: 5, minZ: 0, maxZ: 0});
			assert.equal(envelope.maxY, 5);
		});
		it('should have "minZ" property', function() {
			var envelope = new gdal.Envelope3D({minX: 0, maxX: 0, minY: 0, maxY: 0, minZ: 5, maxZ: 0});
			assert.equal(envelope.minZ, 5);
		});
		it('should have "maxZ" property', function() {
			var envelope = new gdal.Envelope3D({minX: 0, maxX: 0, minY: 0, maxY: 0, minZ: 0, maxZ: 5});
			assert.equal(envelope.maxZ, 5);
		});
		describe('isEmpty()', function() {
			it('should return true when all components zero', function() {
				var envelope = new gdal.Envelope3D();
				assert.isTrue(envelope.isEmpty());
			});
			it('should return false when a component is non-zero', function() {
				var envelope = new gdal.Envelope3D({minX: 0, maxX: 0, minY: 0, maxY: 0, minZ: 10, maxZ: 20});
				assert.isFalse(envelope.isEmpty());
			});
		});
		describe('merge()', function() {
			describe('w/x,y,z arguments', function() {
				it('should expand envelope', function() {
					var envelope = new gdal.Envelope3D({minX: -1, maxX: 1, minY: -2, maxY: 2, minZ: 0, maxZ: 1});
					envelope.merge(2, 3, 5);
					assert.equal(envelope.minX, -1);
					assert.equal(envelope.minY, -2);
					assert.equal(envelope.minZ, 0);
					assert.equal(envelope.maxX, 2);
					assert.equal(envelope.maxY, 3);
					assert.equal(envelope.maxZ, 5);
				});
			});
			describe('w/envelope argument', function() {
				it('should expand envelope', function() {
					var envelopeA = new gdal.Envelope3D({minX: -1, maxX: 1, minY: -2, maxY: 2, minZ: 0, maxZ: 1});
					var envelopeB = new gdal.Envelope3D({minX: -2, maxX: 10, minY: -1, maxY: 1, minZ: -1, maxZ: 2});
					envelopeA.merge(envelopeB);
					assert.equal(envelopeA.minX, -2);
					assert.equal(envelopeA.minY, -2);
					assert.equal(envelopeA.minZ, -1);
					assert.equal(envelopeA.maxX, 10);
					assert.equal(envelopeA.maxY, 2);
					assert.equal(envelopeA.maxZ, 2);
				});
			});
		});
		describe('intersects()', function() {
			it('should determine if envelopes touch', function() {
				var envelopeA = new gdal.Envelope3D({minX: 1, maxX: 2, minY: 1, maxY: 2, minZ: 0, maxZ: 1});
				var envelopeB = new gdal.Envelope3D({minX: 2, maxX: 4, minY: 1, maxY: 2, minZ: 0.5, maxZ: 2});
				var envelopeC = new gdal.Envelope3D({minX: 1, maxX: 2, minY: 1, maxY: 2, minZ: 2, maxZ: 3});
				assert.isTrue(envelopeA.intersects(envelopeB));
				assert.isFalse(envelopeA.intersects(envelopeC));
			});
		});
		describe('contains()', function() {
			it('should determine if it fully contains the other envelope', function() {
				var envelopeA = new gdal.Envelope3D({minX: -10, maxX: 10, minY: -10, maxY: 10, minZ: -10, maxZ: 10});
				var envelopeB = new gdal.Envelope3D({minX: -1, maxX: 1, minY: -1, maxY: 1, minZ: -1, maxZ: 1});
				var envelopeC = new gdal.Envelope3D({minX: -1, maxX: 1, minY: -1, maxY: 1, minZ: -20, maxZ: 1});
				assert.isTrue(envelopeA.contains(envelopeB));
				assert.isFalse(envelopeA.contains(envelopeC));
			});
		});
		describe('intersect()', function() {
			it('should yield empty envelope if the two don\'t intersect', function() {
				var envelopeA = new gdal.Envelope3D({minX: 1, maxX: 2, minY: 1, maxY: 2, minZ: -10, maxZ: 10});
				var envelopeC = new gdal.Envelope3D({minX: 10, maxX: 20, minY: 10, maxY: 20, minZ: -1, maxZ: 2});
				envelopeA.intersect(envelopeC);
				assert.equal(envelopeA.minX, 0);
				assert.equal(envelopeA.maxX, 0);
				assert.equal(envelopeA.minY, 0);
				assert.equal(envelopeA.maxY, 0);
			});
			it('should yield other envelope if current envelope is empty and other envelope intersects 0,0,0', function() {
				var envelopeA = new gdal.Envelope3D();
				var envelopeC = new gdal.Envelope3D({minX: -10, maxX: 20, minY: -10, maxY: 20, minZ: -10, maxZ: 20});
				envelopeA.intersect(envelopeC);
				assert.equal(envelopeA.minX, -10);
				assert.equal(envelopeA.maxX, 20);
				assert.equal(envelopeA.minY, -10);
				assert.equal(envelopeA.maxY, 20);
				assert.equal(envelopeA.minZ, -10);
				assert.equal(envelopeA.maxZ, 20);
			});
			it('should otherwise yield intersection', function() {
				var envelopeA = new gdal.Envelope3D({minX: -10, maxX: 10, minY: -10, maxY: 10, minZ: -10, maxZ: 10});
				var envelopeB = new gdal.Envelope3D({minX: -2, maxX: 12, minY: -1, maxY: 1, minZ: -2, maxZ: 2});
				envelopeA.intersect(envelopeB);
				assert.equal(envelopeA.minX, -2);
				assert.equal(envelopeA.maxX, 10);
				assert.equal(envelopeA.minY, -1);
				assert.equal(envelopeA.maxY, 1);
				assert.equal(envelopeA.minZ, -2);
				assert.equal(envelopeA.maxZ, 2);
			});
		});
	});
});