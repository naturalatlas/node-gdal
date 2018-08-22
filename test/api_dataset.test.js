var fs = require('fs');
var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;
var fileUtils = require('./utils/file.js');

var NAD83_WKT = 'PROJCS["NAD_1983_UTM_Zone_10N",' +
			    'GEOGCS["GCS_North_American_1983",' +
			    'DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137,298.257222101]],' +
			    'PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],' +
			    'PROJECTION["Transverse_Mercator"],PARAMETER["False_Easting",500000.0],' +
			    'PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",-123.0],' +
			    'PARAMETER["Scale_Factor",0.9996],PARAMETER["Latitude_of_Origin",0.0],' +
			    'UNIT["Meter",1.0]]';

describe('gdal.Dataset', function() {
	afterEach(gc);

	var ds;
	before(function() {
		ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
	});
	it('should be exposed', function() {
		assert.ok(gdal.Dataset);
	});
	it('should not be instantiable', function() {
		assert.throws(function() {
			new gdal.Dataset();
		}, /Cannot create dataset directly/);
	});

	describe('instance', function() {
		describe('"bands" property', function() {
			it('should exist', function() {
				assert.instanceOf(ds.bands, gdal.DatasetBands);
			});
			describe('count()', function() {
				it('should return number', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.equal(ds.bands.count(), 1);
				});
				it('should be 0 for vector datasets', function() {
					var arr = [];
					for (var i = 0; i < 10000; i++) arr.push(i);
					console.log('before');
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					console.log('after');
					assert.equal(ds.bands.count(), 0);
				});
				it('should throw if dataset is closed', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					ds.close();
					assert.throws(function() {
						ds.bands.count();
					});
				});
			});
			describe('get()', function() {
				it('should return RasterBand', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.instanceOf(ds.bands.get(1), gdal.RasterBand);
				});
				it('should return null if band id is out of range', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.isNull(ds.bands.get(0));
				});
				it('should throw if dataset is closed', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					ds.close();
					assert.throws(function() {
						ds.bands.get(1);
					});
				});
			});
			describe('forEach()', function() {
				it('should call callback for each RasterBand', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					var expected_ids = [1];
					var ids = [];
					ds.bands.forEach(function(band, i) {
						assert.isNumber(i);
						assert.isTrue(i > 0);
						assert.instanceOf(band, gdal.RasterBand);
						ids.push(band.id);
					});
					assert.deepEqual(ids, expected_ids);
				});
				it('should throw if dataset is closed', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					ds.close();
					assert.throws(function() {
						ds.bands.forEach(function() {});
					});
				});
			});
			describe('map()', function() {
				it('should operate normally', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					var result = ds.bands.map(function(band, i) {
						assert.isNumber(i);
						assert.instanceOf(band, gdal.RasterBand);
						return 'a';
					});

					assert.isArray(result);
					assert.equal(result[0], 'a');
					assert.equal(result.length, ds.bands.count());
				});
			});
		});
		describe('"layers" property', function() {
			it('should exist', function() {
				assert.instanceOf(ds.layers, gdal.DatasetLayers);
			});
			describe('count()', function() {
				it('should return number', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.equal(ds.layers.count(), 1);
				});
				it('should be 0 for raster datasets', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.equal(ds.layers.count(), 0);
				});
				it('should throw if dataset is closed', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					ds.close();
					assert.throws(function() {
						ds.layers.count();
					});
				});
			});
			describe('get()', function() {
				describe('w/id argument', function() {
					it('should return Layer', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						assert.instanceOf(ds.layers.get(0), gdal.Layer);
					});
					it('should return null if layer id is out of range', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						assert.isNull(ds.layers.get(5));
					});
					it('should throw if dataset is closed', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						ds.close();
						assert.throws(function() {
							ds.layers.get(0);
						});
					});
				});
				describe('w/name argument', function() {
					it('should return Layer', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						assert.instanceOf(ds.layers.get('sample'), gdal.Layer);
					});
					it('should return null if layer name doesnt exist', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						assert.isNull(ds.layers.get('bogus'));
					});
					it('should throw if dataset is closed', function() {
						var ds = gdal.open(__dirname + '/data/shp/sample.shp');
						ds.close();
						assert.throws(function() {
							ds.layers.get('sample');
						});
					});
				});
			});
			describe('forEach()', function() {
				it('should call callback for each Layer', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					var expected_names = ['sample'];
					var names = [];
					ds.layers.forEach(function(layer, i) {
						assert.isNumber(i);
						assert.instanceOf(layer, gdal.Layer);
						names.push(layer.name);
					});
					assert.deepEqual(names, expected_names);
				});
				it('should throw if dataset is closed', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					ds.close();
					assert.throws(function() {
						ds.layers.forEach(function() {});
					});
				});
			});
			describe('map()', function() {
				it('should operate normally', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					var result = ds.layers.map(function(layer, i) {
						assert.isNumber(i);
						assert.instanceOf(layer, gdal.Layer);
						return 'a';
					});

					assert.isArray(result);
					assert.equal(result[0], 'a');
					assert.equal(result.length, ds.layers.count());
				});
			});
			describe('create()', function() {
				it('should return Layer', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var srs = gdal.SpatialReference.fromEPSG(4326);
					var lyr = ds.layers.create('layer_name', srs, gdal.wkbPoint);
					assert.instanceOf(lyr, gdal.Layer);
					assert.equal(lyr.geomType, gdal.wkbPoint);
				});
				it('should set spatial reference of created layer', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var srs = gdal.SpatialReference.fromEPSG(4326);
					var lyr = ds.layers.create('layer_name', srs, gdal.wkbPoint);
					assert.instanceOf(lyr.srs, gdal.SpatialReference);
				});
				it('should accept null for srs', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var lyr = ds.layers.create('layer_name', null, gdal.wkbPoint);
					assert.instanceOf(lyr, gdal.Layer);
				});
				it('should accept Geometry constructor for geom_type', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var lyr = ds.layers.create('layer_name', null, gdal.Point);
					assert.instanceOf(lyr, gdal.Layer);
					assert.equal(lyr.geomType, gdal.wkbPoint);
				});
				it('should accept 2.5D Types for geom_type', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var lyr = ds.layers.create('layer_name', null, gdal.wkbPoint25D);
					assert.instanceOf(lyr, gdal.Layer);
					assert.equal(lyr.geomType, gdal.wkbPoint25D);

					file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					ds = gdal.open(file, 'w', 'ESRI Shapefile');
					lyr = ds.layers.create('layer_name', null, gdal.wkbPoint | gdal.wkb25DBit);
					assert.instanceOf(lyr, gdal.Layer);
					assert.equal(lyr.geomType, gdal.wkbPoint25D);
				});
				it('should throw if bad geometry type is given', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					assert.throws(function() {
						ds.layers.create('layer_name', null, console.log);
					});
					assert.throws(function() {
						ds.layers.create('layer_name', null, 16819189);
					});
				});
				it('should error if dataset doesnt support creating layers', function() {
					ds = gdal.open(fileUtils.clone(__dirname + '/data/park.geo.json'), 'r');
					assert.throws(function() {
						ds.layers.create('layer_name', null, gdal.wkbPoint);
					});
				});
				it('should accept layer creation options', function() {
					var basename = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2);
					var file = basename + '.dbf';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					var lyr = ds.layers.create('layer_name', null, null, ['SHPT=NULL']);
					assert.instanceOf(lyr, gdal.Layer);
					ds.close();
					// check if .dbf file was created
					fs.statSync(file);
					// make sure that .shp file wasnt created
					assert.throws(function() {
						fs.statSync(basename + '.shp');
					});
				});
				it('should throw if dataset is closed', function() {
					var file = __dirname + '/data/temp/ds_layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
					var ds = gdal.open(file, 'w', 'ESRI Shapefile');
					ds.close();
					assert.throws(function() {
						ds.layers.create('layer_name', null, gdal.wkbPoint);
					});
				});
			});
		});
		describe('"srs" property', function() {
			describe('getter', function() {
				it('should return SpatialReference', function() {
					var ds;
					ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					assert.ok(ds.srs.toWKT().indexOf('PROJCS["WGS_1984_Albers"') > -1);
				});
				it('should return null when dataset doesn\'t have projection', function() {
					var ds;
					ds = gdal.open(__dirname + '/data/blank.jpg');
					assert.isNull(ds.srs);

					ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.isNull(ds.srs);
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					ds.close();
					assert.throws(function() {
						console.log(ds.srs);
					});
				});
			});
			describe('setter', function() {
				it('should throw when not an SpatialReference object', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.throws(function() {
						ds.srs = '`1`inoinawfawfian!@121';
					}, /srs must be SpatialReference object/);
				});
				it('should set projection', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/dem_azimuth50_pa.img'));

					ds.srs = gdal.SpatialReference.fromWKT(NAD83_WKT);
					assert.equal(ds.srs.toWKT(), NAD83_WKT);
				});
				it('should throw error if dataset doesnt support setting srs', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.throws(function() {
						ds.srs = gdal.SpatialReference.fromWKT(NAD83_WKT);
					});
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/dem_azimuth50_pa.img'));
					ds.close();

					assert.throws(function() {
						ds.srs = gdal.SpatialReference.fromWKT(NAD83_WKT);
					});
				});
			});
		});

		describe('"rasterSize" property', function() {
			describe('getter', function() {
				it('should return dataset dimensions', function() {
					var ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					assert.deepEqual(ds.rasterSize, {
						x: 495,
						y: 286
					});
				});
				it('should return null if dataset isnt a raster', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.isNull(ds.rasterSize);
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					ds.close();
					assert.throws(function() {
						console.log(ds.rasterSize);
					});
				});
			});
			describe('setter', function() {
				it('should throw', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.throws(function() {
						ds.rasterSize = {x: 0, y: 0};
					}, /rasterSize is a read\-only property/);
				});
			});
		});

		describe('"driver" property', function() {
			describe('getter', function() {
				it('should return Driver instance', function() {
					var ds;
					ds = gdal.open(__dirname + '/data/sample.tif');
					assert.instanceOf(ds.driver, gdal.Driver);
					assert.equal(ds.driver.description, 'GTiff');

					ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.instanceOf(ds.driver, gdal.Driver);
					assert.equal(ds.driver.description, 'ESRI Shapefile');
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					ds.close();
					assert.throws(function() {
						console.log(ds.driver);
					});
				});
			});
			describe('setter', function() {
				it('should throw', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					assert.throws(function() {
						ds.driver = null;
					});
				});
			});
		});

		describe('"geoTransform" property', function() {
			describe('getter', function() {
				it('should return array', function() {
					var ds = gdal.open(__dirname + '/data/sample.tif');
					var expected_geotransform = [
						-1134675.2952829634,
						7.502071930146189,
						0,
						2485710.4658232867,
						0,
						-7.502071930145942
					];

					var actual_geotransform = ds.geoTransform;
					var delta = 0.00001;
					assert.closeTo(actual_geotransform[0], expected_geotransform[0], delta);
					assert.closeTo(actual_geotransform[1], expected_geotransform[1], delta);
					assert.closeTo(actual_geotransform[2], expected_geotransform[2], delta);
					assert.closeTo(actual_geotransform[3], expected_geotransform[3], delta);
					assert.closeTo(actual_geotransform[4], expected_geotransform[4], delta);
					assert.closeTo(actual_geotransform[5], expected_geotransform[5], delta);
				});
				it('should return null if dataset doesnt have geotransform', function() {
					var ds = gdal.open(__dirname + '/data/shp/sample.shp');
					assert.isNull(ds.geoTransform);
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					ds.close();
					assert.throws(function() {
						console.log(ds.geoTransform);
					});
				});
			});
			describe('setter', function() {
				it('should set geotransform', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.vrt'));

					var transform = [0, 2, 0, 0, 0, 2];
					ds.geoTransform = transform;
					assert.deepEqual(ds.geoTransform, transform);
				});
				it('should throw if dataset doesnt support setting geotransform', function() {
					var ds;
					var transform = [0, 2, 0, 0, 0, 2];

					ds = gdal.open(fileUtils.clone(__dirname + '/data/park.geo.json'));
					assert.throws(function() {
						ds.geoTransform = transform;
					});

					ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'));
					assert.throws(function() {
						ds.geoTransform = transform;
					});
				});
				it('should throw if dataset is already closed', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.vrt'));
					ds.close();

					var transform = [0, 2, 0, 0, 0, 2];
					assert.throws(function() {
						ds.geoTransform = transform;
					});
				});
				it('should throw if geotransform is invalid', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.vrt'));
					assert.throws(function() {
						ds.geoTransform = [0, 1, 'bad_value', 0, 0, 1];
					});
					assert.throws(function() {
						ds.geoTransform = [0, 1];
					});
				});
			});
		});
		describe('executeSQL()', function() {
			it('should return Layer', function() {
				var ds = gdal.open(__dirname + '/data/shp/sample.shp');
				var result_set = ds.executeSQL('SELECT name FROM sample');

				assert.instanceOf(result_set, gdal.Layer);
				assert.deepEqual(result_set.fields.getNames(), ['name']);
			});
			it('should destroy result set when dataset is closed', function() {
				var ds = gdal.open(__dirname + '/data/shp/sample.shp');
				var result_set = ds.executeSQL('SELECT name FROM sample');
				ds.close();
				assert.throws(function() {
					result_set.fields.getNames();
				});
			});
			it('should throw if dataset already closed', function() {
				var ds = gdal.open(__dirname + '/data/sample.vrt');
				ds.close();
				assert.throws(function() {
					 ds.executeSQL('SELECT name FROM sample');
				});
			});
		});
		describe('getFileList()', function() {
			it('should return list of filenames', function() {
				var ds = gdal.open(path.join(__dirname, 'data', 'sample.vrt'));
				var expected_filenames = [
					ds.description,
					path.join(__dirname, 'data', 'sample.tif')
				];
				assert.deepEqual(ds.getFileList(), expected_filenames);
			});
			it('should throw if dataset already closed', function() {
				var ds = gdal.open(__dirname + '/data/sample.vrt');
				ds.close();
				assert.throws(function() {
					ds.getFileList();
				});
			});
		});
		describe('getMetadata()', function() {
			it('should return object', function() {
				var ds = gdal.open(__dirname + '/data/sample.tif');
				var metadata = ds.getMetadata();
				assert.isObject(metadata);
				assert.equal(metadata.AREA_OR_POINT, 'Area');
			});
			it('should throw if dataset already closed', function() {
				var ds = gdal.open(__dirname + '/data/sample.tif');
				ds.close();
				assert.throws(function() {
					ds.getMetadata();
				});
			});
		});
		describe('buildOverviews()', function() {
			it('should generate overviews for all bands', function() {
				var ds = gdal.open(fileUtils.clone(__dirname + '/data/multiband.tif'), 'r+');
				var expected_w = [ds.rasterSize.x / 2, ds.rasterSize.x / 4, ds.rasterSize.x / 8];
				ds.buildOverviews('NEAREST', [2, 4, 8]);
				ds.bands.forEach(function(band) {
					var w = [];
					assert.equal(band.overviews.count(), 3);
					band.overviews.forEach(function(overview) {
						w.push(overview.size.x);
					});
					assert.sameMembers(w, expected_w);
				});
				ds.close();
			});
			it('should not fail hard if invalid overview is given', function() {
				// 1.11 introduced an error for this, but 1.10 and lower
				// fail silently - so really all we can do is make sure
				// nothing fatal (segfault, etc) happens
				var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'), 'r+');
				try {
					ds.buildOverviews('NEAREST', [2, 4, -3]);
				} catch (e) {
					/* ignore (see above) */
				}
			});
			it('should throw if overview is not a number', function() {
				var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'), 'r+');
				assert.throws(function() {
					ds.buildOverviews('NEAREST', [2, 4, {}]);
				});
			});
			describe('w/bands argument', function() {
				it('should generate overviews only for the given bands', function() {
					gdal.config.set('USE_RRD', 'YES');
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/multiband.tif'), 'r+');
					ds.buildOverviews('NEAREST', [2, 4, 8], [1]);
					assert.equal(ds.bands.get(1).overviews.count(), 3);
				});
				it('should throw if invalid band given', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'), 'r+');
					assert.throws(function() {
						ds.buildOverviews('NEAREST', [2, 4, 8], [4]);
					});
				});
				it('should throw if band id is not a number', function() {
					var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'), 'r+');
					assert.throws(function() {
						ds.buildOverviews('NEAREST', [2, 4, 8], [{}]);
					});
				});
			});
			it('should throw if dataset already closed', function() {
				var ds = gdal.open(fileUtils.clone(__dirname + '/data/sample.tif'), 'r+');
				ds.close();
				assert.throws(function() {
					ds.buildOverviews('NEAREST', [2, 4, 8]);
				});
			});
		});
	});
	describe('setGCPs()', function() {
		it('should update gcps', function() {
			var driver = gdal.drivers.get('MEM');
			var outputFilename = ''; // __dirname + '/data/12_791_1476.tif';
			var ds = driver.createCopy(outputFilename, gdal.open(__dirname + '/data/12_791_1476.jpg'));
			ds.srs = gdal.SpatialReference.fromEPSG(4326);
			var bounds = {
				minX: -110.478515625,
				maxX: -110.390625,
				minY: 44.77793589631623,
				maxY: 44.84029065139799
			};
			var expectedGCPs = [{
				dfGCPPixel: 0,
				dfGCPLine: 0,
				dfGCPX: bounds.minX,
				dfGCPY: bounds.maxY,
				dfGCPZ: 0
			}, {
				dfGCPPixel: 255,
				dfGCPLine: 0,
				dfGCPX: bounds.maxX,
				dfGCPY: bounds.maxY,
				dfGCPZ: 0
			}, {
				dfGCPPixel: 255,
				dfGCPLine: 255,
				dfGCPX: bounds.maxX,
				dfGCPY: bounds.minY,
				dfGCPZ: 0
			}, {
				dfGCPPixel: 0,
				dfGCPLine: 255,
				dfGCPX: bounds.minX,
				dfGCPY: bounds.minY,
				dfGCPZ: 0
			}];

			ds.setGCPs(expectedGCPs);
			var actualGCPs = ds.getGCPs();

			expectedGCPs.forEach(function(expectedGCP, i) {
				var actualGCP = actualGCPs[i];
				var delta = 0.00001;
				assert.closeTo(actualGCP.dfGCPLine, expectedGCP.dfGCPLine, delta);
				assert.closeTo(actualGCP.dfGCPPixel, expectedGCP.dfGCPPixel, delta);
				assert.closeTo(actualGCP.dfGCPX, expectedGCP.dfGCPX, delta);
				assert.closeTo(actualGCP.dfGCPY, expectedGCP.dfGCPY, delta);
				assert.closeTo(actualGCP.dfGCPZ, expectedGCP.dfGCPZ, delta);
			});

			ds.close();
		});
	});
});
