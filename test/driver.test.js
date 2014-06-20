'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('Drivers', function() {

	describe('get()', function() {
		it('should return Driver instance', function() {
			assert.instanceOf(gdal.drivers.get('GTiff'), gdal.Driver);
		});
		it('should return null when not found', function() {
			assert.equal(gdal.drivers.get('awfag'), null);
		});
	});

	describe('(raster)', function() {
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
			describe(o, function() {
				it('should exist', function() {
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
		});
	});

	describe('(vector)', function() {
		var drivers = [
			"GEOJSON", "VRT:vector", "ESRI Shapefile"
		];

		drivers.forEach(function(o) {
			describe(o + ' (OGR)', function() {
				it('should exist', function() {
					assert.ok(gdal.drivers.get(o));
				});
			});
		});
	});
});
