var fs = require('fs');
var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

// http://epsg.io/
// http://spatialreference.org/ref/

describe('gdal.SpatialReference', function() {
	afterEach(gc);

	it('should be exposed', function() {
		assert.ok(gdal.SpatialReference);
	});
	it('should be instantiable', function() {
		assert.instanceOf(new gdal.SpatialReference(), gdal.SpatialReference);
	});
	describe('fromWKT()', function() {
		it('should return SpatialReference', function() {
			var wkt = 'PROJCS["NAD_1983_UTM_Zone_10N",' +
				'GEOGCS["GCS_North_American_1983",' +
				'DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137,298.257222101]],' +
				'PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],' +
				'PROJECTION["Transverse_Mercator"],PARAMETER["False_Easting",500000.0],' +
				'PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",-123.0],' +
				'PARAMETER["Scale_Factor",0.9996],PARAMETER["Latitude_of_Origin",0.0],' +
				'UNIT["Meter",1.0]]';

			var ref = gdal.SpatialReference.fromWKT(wkt);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('fromProj4()', function() {
		it('should return SpatialReference', function() {
			var proj = '+proj=stere +lat_ts=-37 +lat_0=-90 +lon_0=145 +k_0=1.0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs';
			var ref = gdal.SpatialReference.fromProj4(proj);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('fromEPSG()', function() {
		it('should return SpatialReference', function() {
			var epsg = 4326;
			var ref = gdal.SpatialReference.fromEPSG(epsg);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('fromEPSGA()', function() {
		it('should return SpatialReference', function() {
			var epsga = 26910;
			var ref = gdal.SpatialReference.fromEPSGA(epsga);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('fromESRI', function() {
		it('should return SpatialReference', function() {
			var esri = ['GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137,298.257222101]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]]'];
			var ref = gdal.SpatialReference.fromESRI(esri);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe.skip('fromXML', function() {
		it('should return SpatialReference', function() {
			var gml = fs.readFileSync(__dirname + '/data/srs/sample.gml', 'utf8');
			var ref = gdal.SpatialReference.fromXML(gml);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('fromWMSAUTO', function() {
		it('should return SpatialReference', function() {
			var wms = 'AUTO:42001,99,8888';
			var ref = gdal.SpatialReference.fromWMSAUTO(wms);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe.skip('fromURN', function() {
		it('should return SpatialReference', function() {
			var wms = 'urn:ogc:def:crs:EPSG::26912';
			var ref = gdal.SpatialReference.fromWMSAUTO(wms);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe.skip('fromCRSURL', function() {
		it('should return SpatialReference', function() {
			var wms = 'CRS:84';
			var ref = gdal.SpatialReference.fromCRSURL(wms);
			assert.instanceOf(ref, gdal.SpatialReference);
		});
	});
	describe('getAuthorityCode', function() {
		it('should support string argument', function() {
			var srs = gdal.SpatialReference.fromUserInput('EPSG:27700');
			assert.strictEqual(srs.getAuthorityCode('PROJCS'), '27700');
		});
		it('should support null argument', function() {
			// https://github.com/naturalatlas/node-gdal/issues/218
			var wkt = 'GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]]';
			var srs = gdal.SpatialReference.fromWKT(wkt);
			srs.autoIdentifyEPSG();
			assert.strictEqual(srs.getAuthorityCode(null), '4269');
		});
		it('should support no arguments', function() {
			var wkt = 'GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]]';
			var srs = gdal.SpatialReference.fromWKT(wkt);
			srs.autoIdentifyEPSG();
			assert.strictEqual(srs.getAuthorityCode(), '4269');
		});
	});
	describe('getAuthorityName', function() {
		it('should support string argument', function() {
			var srs = gdal.SpatialReference.fromUserInput('EPSG:27700');
			assert.strictEqual(srs.getAuthorityName('PROJCS'), 'EPSG');
		});
		it('should support null argument', function() {
			var wkt = 'GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]]';
			var srs = gdal.SpatialReference.fromWKT(wkt);
			srs.autoIdentifyEPSG();
			assert.strictEqual(srs.getAuthorityName(null), 'EPSG');
		});
		it('should support no arguments', function() {
			var wkt = 'GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]]';
			var srs = gdal.SpatialReference.fromWKT(wkt);
			srs.autoIdentifyEPSG();
			assert.strictEqual(srs.getAuthorityName(), 'EPSG');
		});
	});
	describe('toProj4', function() {
		it('should return string', function() {
			var srs = gdal.SpatialReference.fromUserInput('NAD83');
			assert.equal(srs.toProj4(), '+proj=longlat +datum=NAD83 +no_defs');
		});
	});
	describe('isGeographic', function() {
		it('should return true if geographic coordinate system', function() {
			assert.equal(gdal.SpatialReference.fromEPSG(4326).isGeographic(), true);
		});
		it('should return false if not geographic coordinate system', function() {
			assert.equal(gdal.SpatialReference.fromEPSG(2154).isGeographic(), false);
		});
	});
});
