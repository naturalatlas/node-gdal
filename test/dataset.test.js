'use strict';

var fs = require('fs');
var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;

describe('Dataset', function() {
	it('should be exposed', function() {
		assert.ok(gdal.Dataset);
	});
	it('should not be instantiable', function() {
		assert.throws(function() {
			new gdal.Dataset();
		}, /Cannot create dataset directly/);
	});

	describe('getProjectionRef()', function() {
		var ds;

		before(function() { ds = gdal.open(__dirname + "/data/dem_azimuth50_pa.img"); });
		after(function() { ds.close(); });

		it('should return projection string', function() {
			assert.equal(ds.getProjectionRef(), 'PROJCS["WGS_1984_Albers",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9108"]],AUTHORITY["EPSG","4326"]],PROJECTION["Albers_Conic_Equal_Area"],PARAMETER["standard_parallel_1",40],PARAMETER["standard_parallel_2",42],PARAMETER["latitude_of_center",39],PARAMETER["longitude_of_center",-78],PARAMETER["false_easting",0],PARAMETER["false_northing",0],UNIT["meters",1]]');
		});
		it('should return "" when dataset doesn\'t have projection', function() {
			var ds = gdal.open(__dirname + "/data/blank.jpg");
			assert.equal(ds.getProjectionRef(), '');
		});
	});

	describe('setProjection()', function() {
		it('should throw when invalid projection', function() {
			var ds = gdal.open(__dirname + "/data/sample.tif");
			assert.throws(function() {
				ds.setProjection('`1`inoinawfawfian!@121');
			}, /not supported/);
		});
		it('should set projection (WKT)', function() {
			var file_orig = __dirname + "/data/dem_azimuth50_pa.img";
			var file_test = __dirname + "/data/dem_azimuth50_pa.tmp1.img";

			fs.writeFileSync(file_test, fs.readFileSync(file_orig));
			var ds = gdal.open(file_test);

			var ref = 'PROJCS["NAD_1983_UTM_Zone_10N",' +
				'GEOGCS["GCS_North_American_1983",' +
				'DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137,298.257222101]],' +
				'PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],' +
				'PROJECTION["Transverse_Mercator"],PARAMETER["False_Easting",500000.0],' +
				'PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",-123.0],' +
				'PARAMETER["Scale_Factor",0.9996],PARAMETER["Latitude_of_Origin",0.0],' +
				'UNIT["Meter",1.0]]';

			ds.setProjection(ref);
			assert.equal(ds.getProjectionRef(), ref);
		});
		it('should set projection (PROJ)', function() {
			var file_orig = __dirname + "/data/dem_azimuth50_pa.img";
			var file_test = __dirname + "/data/dem_azimuth50_pa.tmp2.img";

			fs.writeFileSync(file_test, fs.readFileSync(file_orig));
			var ds = gdal.open(file_test);
			var ref = '+proj=stere +lat_ts=-37 +lat_0=-90 +lon_0=145 +k_0=1.0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs';

			ds.setProjection(ref);
			assert.equal(ds.getProjectionRef(), ref);
		});
	});

	describe('getRasterBand()', function() {
		var ds;
		before(function() {
			ds = gdal.open(__dirname + "/data/sample.tif");
		});

		it('should throw when invalid band', function() {
			assert.throws(function() {
				ds.getRasterBand(2);
			}, /band not found/);
		});
		it('should return RasterBand instance when valid band', function() {
			var band = ds.getRasterBand(1);
			assert.instanceOf(band, gdal.RasterBand);
		});
	});

});