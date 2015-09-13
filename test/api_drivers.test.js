'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe.skip('gdal.drivers', function() {
	afterEach(gc);

	describe('count()', function() {
		it('should return the number of drivers', function() {
			assert.isNumber(gdal.drivers.count());
		});
	});

	describe('getNames()', function() {
		it('should return array of strings', function() {
			var result = gdal.drivers.getNames();
			assert.isArray(result);
			result.forEach(function(driver) {
				assert.isString(driver);
			});
		});
		it('should return array the same length as driver count', function() {
			var count = gdal.drivers.count();
			assert.lengthOf(gdal.drivers.getNames(), count);
		});
	});

	describe('get()', function() {
		it('should support numeric argument', function() {
			assert.instanceOf(gdal.drivers.get(0), gdal.Driver);
		});
		it('should support string argument', function() {
			assert.instanceOf(gdal.drivers.get('GTiff'), gdal.Driver);
		});
		it('should return null when not found', function() {
			assert.equal(gdal.drivers.get('awfag'), null);
		});

		// raster drivers
		var expected = {
			"GTiff": {
				DMD_LONGNAME: "GeoTIFF",
				DMD_MIMETYPE: "image/tiff",
				DMD_EXTENSION: "tif",
				DCAP_CREATE: 'YES'
			},
			"VRT:raster": {
				DMD_LONGNAME: "Virtual Raster",
				DMD_MIMETYPE: undefined,
				DMD_EXTENSION: "vrt",
				DCAP_CREATE: 'YES'
			},
			"MEM": {
				DMD_LONGNAME: "In Memory Raster",
				DMD_MIMETYPE: undefined,
				DMD_EXTENSION: undefined,
				DCAP_CREATE: 'YES'
			},
			"PNG": {
				DMD_LONGNAME: "Portable Network Graphics",
				DMD_MIMETYPE: "image/png",
				DMD_EXTENSION: "png",
				DCAP_CREATE: undefined
			},
			"JPEG": {
				DMD_LONGNAME: "JPEG JFIF",
				DMD_MIMETYPE: "image/jpeg",
				DMD_EXTENSION: "jpg",
				DCAP_CREATE: undefined
			}
		};

		Object.keys(expected).forEach(function(o) {
			it('should support "' + o + '" driver', function() {
				var driver = gdal.drivers.get(o);
				assert.ok(driver);

				var metadata = driver.getMetadata();
				var expected_meta = expected[o];
				assert.equal(expected_meta.DMD_LONGNAME,metadata.DMD_LONGNAME);
				assert.equal(expected_meta.DMD_MIMETYPE,metadata.DMD_MIMETYPE);
				assert.equal(expected_meta.DMD_EXTENSION,metadata.DMD_EXTENSION);
				assert.equal(expected_meta.DCAP_CREATE,metadata.DCAP_CREATE);
			});
		});

		// vector drivers
		var drivers = [
			"GEOJSON", "VRT:vector", "ESRI Shapefile"
		];

		drivers.forEach(function(o) {
			it('should support "' + o + '" (ogr) driver', function() {
				it('should exist', function() {
					assert.ok(gdal.drivers.get(o));
				});
			});
		});
	});

	describe('forEach()', function() {
		it('should iterate through all Driver objects', function() {
			var n = gdal.drivers.count();
			var i = 0;
			gdal.drivers.forEach(function(driver) {
				assert.instanceOf(driver, gdal.Driver);
				assert.equal(driver, gdal.drivers.get(i++));
			});
			assert.equal(i, n);
		});
		it('should stop when false is returned from callback', function() {
			var i = 0;
			gdal.drivers.forEach(function() { if (++i === 1) return false; });
			assert.equal(i, 1);
		});
		it('should always start from beginning', function() {
			var n = gdal.drivers.count();
			var i = 0, j = 0;
			gdal.drivers.forEach(function() { if (i++ === 1) return false; });
			gdal.drivers.forEach(function() { j++; });
			assert.equal(j, n);
		});
	});
});
