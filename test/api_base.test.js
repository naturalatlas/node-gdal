'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;
var fs = require('fs');
var path = require('path');

if (process.env.GDAL_DATA !== undefined) {
	throw new Error("Sorry, this test requires that the GDAL_DATA environment option is not set");
}

describe('gdal', function() {
	afterEach(gc);

	describe('"lastError" property', function() {
		describe('get()', function() {
			it('should return null when no previous error', function() {
				// note: this needs to be the first test run
				assert.isNull(gdal.lastError);
			});
			it('should return an object normally', function() {
				gdal._triggerCPLError();

				assert.deepEqual(gdal.lastError, {
					code: gdal.CPLE_AppDefined,
					level: gdal.CE_Failure,
					message: 'Mock error'
 				});
			});
		});
		describe('set()', function() {
			it('should allow reset by setting to null', function() {
				gdal._triggerCPLError();

				assert.equal(!!gdal.lastError, true);
				gdal.lastError = null;
				assert.isNull(gdal.lastError);
			});
			it('should throw when not null', function() {
				assert.throws(function() {
					gdal.lastError = {};
				}, /null/);
			});
		});
	});
	describe('"version" property', function() {
		it('should exist', function() {
			assert.match(gdal.version, /^\d+\.\d+\.\d+[a-zA-Z]*$/);
		});
	});
	describe('"config" property', function() {
		describe('get()', function() {
			it('should not throw', function() {
				gdal.config.get('CPL_LOG');
			});
		});
		describe('set()', function() {
			it('should set option', function() {
				gdal.config.set('CPL_DEBUG', 'ON');
				assert.equal(gdal.config.get('CPL_DEBUG'), 'ON');
				gdal.config.set('CPL_DEBUG', null);
				assert.isNull(gdal.config.get('CPL_DEBUG'));
			});
		});
		describe('GDAL_DATA behavior', function() {
			var data_path = path.resolve(__dirname, '../deps/libgdal/gdal/data');
			it('should set GDAL_DATA config option to locally bundled path', function() {
				assert.equal(gdal.config.get('GDAL_DATA'),data_path);
			});
			it('should respect GDAL_DATA environment over locally bundled path', function(done) {
				process.env.GDAL_DATA = 'bogus';
				var cp = require('child_process');
				var command = "\"var gdal = require('./lib/gdal.js'); console.log(gdal.config.get('GDAL_DATA'));\"";
				var execPath = process.execPath;
				if (process.platform === 'win32') {
					// attempt to avoid quoting problem that leads to error like ''C:\Program' is not recognized as an internal or external command'
					execPath = '"' + execPath + '"';
				}
				cp.exec(execPath + ' ' + ['-e',command].join(' '),{env:{GDAL_DATA:'bogus'}},function(err,stdout,stderr) {
					if (err) throw err;
					assert.equal(process.env.GDAL_DATA,stdout.trim());
					done();
				})
			});
		});
	});
	describe('decToDMS()', function() {
		it('should throw when axis not provided', function() {
			assert.throws(function() {
				gdal.decToDMS(12.2);
			});
		});
		it('should return correct result', function() {
			assert.equal(gdal.decToDMS(14.12511, 'lat', 2), ' 14d 7\'30.40"N');
			assert.equal(gdal.decToDMS(14.12511, 'lat', 1), ' 14d 7\'30.4"N');
			assert.equal(gdal.decToDMS(14.12511, 'long', 2), ' 14d 7\'30.40"E');
			assert.equal(gdal.decToDMS(14.12511, 'long', 1), ' 14d 7\'30.4"E');
		});
	});
});
