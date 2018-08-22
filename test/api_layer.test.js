var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;
var fileUtils = require('./utils/file.js');

describe('gdal.Layer', function() {
	afterEach(gc);

	describe('instance', function() {
		var prepare_dataset_layer_test = function() {
			var ds, layer, mode, options, callback, err, file, dir, driver;

			if (arguments.length === 2) {
				mode = arguments[0];
				options = {};
				callback = arguments[1];
			} else {
				mode = arguments[0];
				options = arguments[1] || {};
				callback = arguments[2];
			}

			// set dataset / layer
			if (mode === 'r') {
				dir = fileUtils.cloneDir(__dirname + '/data/shp');
				file = dir + '/sample.shp';
				ds = gdal.open(file);
				layer = ds.layers.get(0);
			} else {
				driver = gdal.drivers.get('ESRI Shapefile');
				file = __dirname + '/data/temp/layer_test.' + String(Math.random()).substring(2) + '.tmp.shp';
				ds = driver.create(file);
				layer = ds.layers.create('layer_test', null, gdal.Point);
			}

			// run test and then teardown
			try {
				callback(ds, layer);
			} catch (e) {
				err = e;
			}

			// teardown
			if (options.autoclose !== false) {
				try {
					ds.close();
				} catch (e) {
					/* ignore */
				}
				if (file && mode === 'w') {
					try {
						driver.deleteDataset(file);
					} catch (e) {
						/* ignore */
					}
				}
			}

			if (err) throw err;
		};

		describe('"ds" property', function() {
			describe('getter', function() {
				it('should return Dataset', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.instanceOf(layer.ds, gdal.Dataset);
						assert.equal(layer.ds, dataset);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							console.log(layer.ds);
						}, /already been destroyed/);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.ds = null;
						}, /ds is a read-only property/);
					});
				});
			});
		});

		describe('"srs" property', function() {
			describe('getter', function() {
				it('should return SpatialReference', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						// EPSG:4269 - exact WKT can vary when using shared GDAL / Proj4 library
						var expectedWKT = [
							'GEOGCS["NAD83",DATUM["North_American_Datum_1983",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6269"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4269"]]',
							'GEOGCS["GCS_North_American_1983",DATUM["North_American_Datum_1983",SPHEROID["GRS_1980",6378137,298.257222101]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]]'
						];
						assert.include(expectedWKT, layer.srs.toWKT());
					});
				});
				it('should return the same SpatialReference object', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var srs1 = layer.srs;
						var srs2 = layer.srs;
						assert.equal(srs1, srs2);
					});
				});
				// NOTE: geojson has a default projection: EPSG 4326
				// it('should return null when dataset doesn\'t have projection', function() {
				// 	var ds = gdal.open(__dirname + "/data/park.geo.json");
				// 	var layer = ds.layers.get(0);
				// 	assert.isNull(layer.srs);
				// });
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							console.log(layer.srs);
						}, /already been destroyed/);
					});
				});
				describe('result', function() {
					it('should not be destroyed when dataset is destroyed', function() {
						prepare_dataset_layer_test('r', function(dataset, layer) {
							var srs = layer.srs;
							dataset.close();
							assert.doesNotThrow(function() {
								assert.ok(srs.toWKT());
							});
						});
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.srs = 'ESPG:4326';
						}, /srs is a read-only property/);
					});
				});
			});
		});

		describe('"name" property', function() {
			describe('getter', function() {
				it('should return string', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.equal(layer.name, 'sample');
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							console.log(layer.name);
						});
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.name = null;
						}, /name is a read-only property/);
					});
				});
			});
		});

		describe('"geomType" property', function() {
			describe('getter', function() {
				it('should return wkbGeometryType', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.equal(layer.geomType, gdal.wkbPolygon);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							console.log(layer.geomType);
						}, /already been destroyed/);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.geomType = null;
						}, /geomType is a read-only property/);
					});
				});
			});
		});

		describe('testCapability()', function() {
			it("should return false when layer doesn't support capability", function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					assert.isFalse(layer.testCapability(gdal.OLCCreateField));
				});
			});
			it('should return true when layer does support capability', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					assert.isTrue(layer.testCapability(gdal.OLCRandomRead));
				});
			});
			it('should throw error if dataset is destroyed', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					dataset.close();
					assert.throws(function() {
						layer.testCapability(gdal.OLCCreateField);
					}, /already been destroyed/);
				});
			});
		});

		describe('getExtent()', function() {
			it('should return Envelope', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var actual_envelope = layer.getExtent();
					var expected_envelope = {
						minX: -111.05687488399991,
						minY: 40.99549316200006,
						maxX: -104.05224885499985,
						maxY: 45.00589722600017
					};

					assert.instanceOf(actual_envelope, gdal.Envelope);
					assert.closeTo(actual_envelope.minX, expected_envelope.minX, 0.00001);
					assert.closeTo(actual_envelope.minY, expected_envelope.minY, 0.00001);
					assert.closeTo(actual_envelope.maxX, expected_envelope.maxX, 0.00001);
					assert.closeTo(actual_envelope.maxY, expected_envelope.maxY, 0.00001);
				});
			});
			it("should throw error if force flag is false and layer doesn't have extent already computed", function() {
				var dataset = gdal.open(__dirname + '/data/park.geo.json');
				var layer = dataset.layers.get(0);
				assert.throws(function() {
					layer.getExtent(false);
				}, "Can't get layer extent without computing it");
			});
			it('should throw error if dataset is destroyed', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					dataset.close();
					assert.throws(function() {
						layer.getExtent();
					}, /already been destroyed/);
				});
			});
		});

		describe('setSpatialFilter()', function() {
			it('should accept 4 numbers', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var count_before = layer.features.count();
					layer.setSpatialFilter(-111, 41, -104, 43);
					var count_after  = layer.features.count();

					assert.isTrue(count_after < count_before, 'feature count has decreased');
				});
			});
			it('should accept Geometry', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var count_before = layer.features.count();
					var filter = new gdal.Polygon();
					var ring   = new gdal.LinearRing();
					ring.points.add(-111, 41);
					ring.points.add(-104, 41);
					ring.points.add(-104, 43);
					ring.points.add(-111, 43);
					ring.points.add(-111, 41);
					filter.rings.add(ring);
					layer.setSpatialFilter(filter);
					var count_after = layer.features.count();

					assert.isTrue(count_after < count_before, 'feature count has decreased');
				});
			});
			it('should clear the spatial filter if passed null', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var count_before = layer.features.count();
					layer.setSpatialFilter(-111, 41, -104, 43);
					layer.setSpatialFilter(null);
					var count_after  = layer.features.count();

					assert.equal(count_before, count_after);
				});
			});
			it('should throw error if dataset is destroyed', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					dataset.close();
					assert.throws(function() {
						layer.setSpatialFilter(-111, 41, -104, 43);
					}, /already been destroyed/);
				});
			});
		});

		describe('getSpatialFilter()', function() {
			it('should return Geometry', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var filter = new gdal.Polygon();
					var ring   = new gdal.LinearRing();
					ring.points.add(-111, 41);
					ring.points.add(-104, 41);
					ring.points.add(-104, 43);
					ring.points.add(-111, 43);
					ring.points.add(-111, 41);
					filter.rings.add(ring);
					layer.setSpatialFilter(filter);

					var result = layer.getSpatialFilter();
					assert.instanceOf(result, gdal.Polygon);
				});
			});
			it('should throw error if dataset is destroyed', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					dataset.close();
					assert.throws(function() {
						layer.getSpatialFilter();
					}, /already been destroyed/);
				});
			});
		});

		describe('setAttributeFilter()', function() {
			it('should filter layer by expression', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var count_before = layer.features.count();
					layer.setAttributeFilter("name = 'Park'");
					var count_after  = layer.features.count();

					assert.isTrue(count_after < count_before, 'feature count has decreased');
				});
			});
			it('should clear the attribute filter if passed null', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					var count_before = layer.features.count();
					layer.setAttributeFilter("name = 'Park'");
					layer.setAttributeFilter(null);
					var count_after  = layer.features.count();

					assert.equal(count_before, count_after);
				});
			});
			it('should throw error if dataset is destroyed', function() {
				prepare_dataset_layer_test('r', function(dataset, layer) {
					dataset.close();
					assert.throws(function() {
						layer.setAttributeFilter("name = 'Park'");
					}, /already been destroyed/);
				});
			});
		});

		describe('"features" property', function() {
			describe('getter', function() {
				it('should return LayerFeatures', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.instanceOf(layer.features, gdal.LayerFeatures);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.features = null;
						}, /features is a read-only property/);
					});
				});
			});
			describe('count()', function() {
				it('should return an integer', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.equal(layer.features.count(), 23);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.count();
						}, /already destroyed/);
					});
				});
			});
			describe('get()', function() {
				it('should return a Feature', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var feature = layer.features.get(0);
						assert.instanceOf(feature, gdal.Feature);
					});
				});
				it('should return null if index doesn\'t exist', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var feature = layer.features.get(99);
						assert.isNull(feature);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.get(0);
						}, /already destroyed/);
					});
				});
			});
			describe('next()', function() {
				it('should return a Feature and increment the iterator', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var f1 = layer.features.next();
						var f2 = layer.features.next();
						assert.instanceOf(f1, gdal.Feature);
						assert.instanceOf(f2, gdal.Feature);
						assert.notEqual(f1, f2);
					});
				});
				it('should return null after last feature', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var count = layer.features.count();
						for (var i = 0; i < count; i++) {
							layer.features.next();
						}
						assert.isNull(layer.features.next());
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.next();
						}, /already destroyed/);
					});
				});
			});
			describe('first()', function() {
				it('should return a Feature and reset the iterator', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						layer.features.next();
						var f = layer.features.first();
						assert.instanceOf(f, gdal.Feature);
						assert.equal(f.fid, 0);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.first();
						}, /already destroyed/);
					});
				});
			});
			describe('forEach()', function() {
				it('should pass each feature to the callback', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var count = 0;
						layer.features.forEach(function(feature, i) {
							assert.isNumber(i);
							assert.instanceOf(feature, gdal.Feature);
							count++;
						});
						assert.equal(count, layer.features.count());
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.forEach(function() {});
						}, /already destroyed/);
					});
				});
			});
			describe('map()', function() {
				it('should operate normally', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var result = layer.features.map(function(feature, i) {
							assert.isNumber(i);
							assert.instanceOf(feature, gdal.Feature);
							return 'a';
						});

						assert.isArray(result);
						assert.equal(result[0], 'a');
						assert.equal(result.length, layer.features.count());
					});
				});
			});
			describe('add()', function() {
				it('should add Feature to layer', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						layer.features.add(new gdal.Feature(layer));
						assert.equal(layer.features.count(), 1);
					});
				});
				it('should throw error if layer doesnt support creating features', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.features.add(new gdal.Feature(layer));
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							var feature = new gdal.Feature(layer);
							layer.features.add(feature);
						}, /already destroyed/);
					});
				});
			});

			describe('set()', function() {
				var f0, f1, f1_new, layer, dataset;
				beforeEach(function() {
					prepare_dataset_layer_test('w', {autoclose: false}, function(ds, lyr) {
						layer = lyr;
						dataset = ds;

						layer.fields.add(new gdal.FieldDefn('status', gdal.OFTString));

						f0 = new gdal.Feature(layer);
						f1 = new gdal.Feature(layer);
						f1_new = new gdal.Feature(layer);

						f0.fields.set('status', 'unchanged');
						f1.fields.set('status', 'unchanged');
						f1_new.fields.set('status', 'changed');

						layer.features.add(f0);
						layer.features.add(f1);
					});
				});
				afterEach(function() {
					try {
						dataset.close();
					} catch (e) {
						/* ignore */
					}
				});

				describe('w/feature argument', function() {
					it('should replace existing feature', function() {
						f1_new.fid = 1;

						assert.equal(layer.features.get(1).fields.get('status'), 'unchanged');
						layer.features.set(f1_new);
						assert.equal(layer.features.get(1).fields.get('status'), 'changed');
					});
				});
				describe('w/fid,feature arguments', function() {
					it('should replace existing feature', function() {
						assert.equal(layer.features.get(1).fields.get('status'), 'unchanged');
						layer.features.set(1, f1_new);
						assert.equal(layer.features.get(1).fields.get('status'), 'changed');
					});
				});
				it('should throw error if layer doesnt support changing features', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.features.set(1, new gdal.Feature(layer));
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.set(1, new gdal.Feature(layer));
						});
					});
				});
			});

			describe('remove()', function() {
				it('should make the feature at fid null', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						layer.features.add(new gdal.Feature(layer));
						layer.features.add(new gdal.Feature(layer));

						assert.instanceOf(layer.features.get(1), gdal.Feature);
						layer.features.remove(1);
						assert.isNull(layer.features.get(1));
					});
				});
				it('should throw error if driver doesnt support deleting features', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.features.remove(1);
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.features.remove(1);
						}, /already destroyed/);
					});
				});
			});
		});

		describe('"fields" property', function() {
			describe('getter', function() {
				it('should return LayerFields', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.instanceOf(layer.fields, gdal.LayerFields);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						assert.throws(function() {
							layer.fields = null;
						}, /fields is a read-only property/);
					});
				});
			});
			describe('count()', function() {
				it('should return an integer', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.equal(layer.fields.count(), 8);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.count();
						}, /already destroyed/);
					});
				});
			});
			describe('get()', function() {
				describe('w/id argument', function() {
					it('should return a FieldDefn', function() {
						prepare_dataset_layer_test('r', function(dataset, layer) {
							var field = layer.fields.get(4);
							assert.instanceOf(field, gdal.FieldDefn);
							assert.equal(field.name, 'fips_num');
						});
					});
				});
				describe('w/name argument', function() {
					it('should return a FieldDefn', function() {
						prepare_dataset_layer_test('r', function(dataset, layer) {
							var field = layer.fields.get('fips_num');
							assert.instanceOf(field, gdal.FieldDefn);
							assert.equal(field.name, 'fips_num');
						});
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.get(4);
						}, /already destroyed/);
					});
				});
			});
			describe('forEach()', function() {
				it('should return pass each FieldDefn to callback', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var expected_names = [
							'path',
							'name',
							'type',
							'long_name',
							'fips_num',
							'fips',
							'state_fips',
							'state_abbr'
						];
						var count = 0;
						layer.fields.forEach(function(field, i) {
							assert.isNumber(i);
							assert.instanceOf(field, gdal.FieldDefn);
							assert.equal(expected_names[i], field.name);
							count++;
						});
						assert.equal(layer.fields.count(), count);
						assert.deepEqual(layer.fields.getNames(), expected_names);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.forEach(function() {});
						}, /already destroyed/);
					});
				});
			});
			describe('map()', function() {
				it('should operate normally', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var result = layer.fields.map(function(field, i) {
							assert.isNumber(i);
							assert.instanceOf(field, gdal.FieldDefn);
							return 'a';
						});

						assert.isArray(result);
						assert.equal(result[0], 'a');
						assert.equal(result.length, layer.fields.count());
					});
				});
			});
			describe('getNames()', function() {
				it('should return an array of field names', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var expected_names = [
							'path',
							'name',
							'type',
							'long_name',
							'fips_num',
							'fips',
							'state_fips',
							'state_abbr'
						];
						assert.deepEqual(layer.fields.getNames(), expected_names);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.getNames();
						}, /already destroyed/);
					});
				});
			});
			describe('indexOf()', function() {
				it('should return index of field name', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						var field_name = layer.fields.get(4).name;
						assert.equal(layer.fields.indexOf(field_name), 4);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.indexOf('fips_num');
						}, /already destroyed/);
					});
				});
			});
			describe('add()', function() {
				describe('w/FieldDefn argument', function() {
					it('should add FieldDefn to layer definition', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							var f0 = new gdal.FieldDefn('field0', gdal.OFTString);
							var f1 = new gdal.FieldDefn('field1', gdal.OFTInteger);
							var f2 = new gdal.FieldDefn('field2', gdal.OFTReal);
							layer.fields.add(f0);
							layer.fields.add(f1);
							layer.fields.add(f2);
							assert.equal(layer.fields.count(), 3);
							assert.equal(layer.fields.get(0).name, 'field0');
							assert.equal(layer.fields.get(1).name, 'field1');
							assert.equal(layer.fields.get(2).name, 'field2');
						});
					});
					it('should throw an error if approx flag is false and layer doesnt support field as it is', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							assert.throws(function() {
								layer.fields.add(new gdal.FieldDefn('some_long_name_over_10_chars', gdal.OFTString), false);
							}, /Failed to add/);
						});
					});
				});
				describe('w/FieldDefn array argument', function() {
					it('should add FieldDefns to layer definition', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							var fields = [
								new gdal.FieldDefn('field0', gdal.OFTString),
								new gdal.FieldDefn('field1', gdal.OFTInteger),
								new gdal.FieldDefn('field2', gdal.OFTReal)
							];
							layer.fields.add(fields);
							assert.equal(layer.fields.count(), 3);
							assert.equal(layer.fields.get(0).name, 'field0');
							assert.equal(layer.fields.get(1).name, 'field1');
							assert.equal(layer.fields.get(2).name, 'field2');
						});
					});
					it('should throw an error if approx flag is false and layer doesnt support field as it is', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							assert.throws(function() {
								layer.fields.add([new gdal.FieldDefn('some_long_name_over_10_chars', gdal.OFTString)], false);
							}, /Failed to add/);
						});
					});
				});
				it('should throw an error if layer doesnt support adding fields', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
						}, /already destroyed/);
					});
				});
			});
			describe('fromObject()', function() {
				it('should make fields from object keys/values', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						var sample_fields = {
							id: 1,
							name: 'some_name',
							value: 3.1415,
							flag: true
						};
						layer.fields.fromObject(sample_fields);
						var f0 = layer.fields.get(0);
						var f1 = layer.fields.get(1);
						var f2 = layer.fields.get(2);
						var f3 = layer.fields.get(3);
						assert.equal(f0.name, 'id');
						assert.equal(f1.name, 'name');
						assert.equal(f2.name, 'value');
						assert.equal(f3.name, 'flag');
						assert.equal(f0.type, gdal.OFTInteger);
						assert.equal(f1.type, gdal.OFTString);
						assert.equal(f2.type, gdal.OFTReal);
						assert.equal(f3.type, gdal.OFTInteger);
					});
				});
				it("should throw error if field name isn't supported", function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						assert.throws(function() {
							layer.fields.fromObject({some_really_long_name: 'test'});
						}, /Failed to add/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						dataset.close();
						assert.throws(function() {
							layer.fields.fromObject({name: 'test'});
						}, /already destroyed/);
					});
				});
			});
			describe('remove()', function() {
				describe('w/id argument', function() {
					it('should remove FieldDefn from layer definition', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
							layer.fields.add(new gdal.FieldDefn('field1', gdal.OFTString));
							assert.equal(layer.fields.count(), 2);

							layer.fields.remove(0);
							assert.equal(layer.fields.count(), 1);
							assert.equal(layer.fields.get(0).name, 'field1');
						});
					});
				});
				describe('w/name argument', function() {
					it('should remove FieldDefn from layer definition', function() {
						prepare_dataset_layer_test('w', function(dataset, layer) {
							layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
							layer.fields.add(new gdal.FieldDefn('field1', gdal.OFTString));
							assert.equal(layer.fields.count(), 2);

							layer.fields.remove('field0');
							assert.equal(layer.fields.count(), 1);
							assert.equal(layer.fields.get(0).name, 'field1');
						});
					});
				});
				it('should throw error if layer doesnt support removing fields', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.fields.remove(0);
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
						dataset.close();
						assert.throws(function() {
							layer.fields.remove(0);
						}, /already destroyed/);
					});
				});
			});
			describe('reorder()', function() {
				it('should reorder fields', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
						layer.fields.add(new gdal.FieldDefn('field1', gdal.OFTString));
						layer.fields.add(new gdal.FieldDefn('field2', gdal.OFTString));

						layer.fields.reorder([2, 0, 1]);
						var f0 = layer.fields.get(0);
						var f1 = layer.fields.get(1);
						var f2 = layer.fields.get(2);
						assert.equal(f0.name, 'field2');
						assert.equal(f1.name, 'field0');
						assert.equal(f2.name, 'field1');
					});
				});
				it('should throw an error if layer doesnt support reordering fields', function() {
					prepare_dataset_layer_test('r', function(dataset, layer) {
						assert.throws(function() {
							layer.fields.reorder([2, 0, 1, 3, 4, 5, 6, 7]);
						}, /read\-only/);
					});
				});
				it('should throw error if dataset is destroyed', function() {
					prepare_dataset_layer_test('w', function(dataset, layer) {
						layer.fields.add(new gdal.FieldDefn('field0', gdal.OFTString));
						layer.fields.add(new gdal.FieldDefn('field1', gdal.OFTString));
						layer.fields.add(new gdal.FieldDefn('field2', gdal.OFTString));
						dataset.close();
						assert.throws(function() {
							layer.fields.reorder([2, 0, 1]);
						}, /already destroyed/);
					});
				});
			});
		});
	});
});
