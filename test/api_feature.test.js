var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal.Feature', function() {
	afterEach(gc);

	var ds, lyr, defn, fields;
	before(function() {
		ds = gdal.open('', 'w', 'Memory');
		lyr = ds.layers.create('', null, gdal.Point);
		fields = [
			new gdal.FieldDefn('id', gdal.OFTInteger),
			new gdal.FieldDefn('name', gdal.OFTString),
			new gdal.FieldDefn('value', gdal.OFTReal)
		];
		defn = new gdal.FeatureDefn();
		defn.fields.add(fields);
		lyr.fields.add(fields);
	});

	describe('constructor', function() {
		describe('w/Layer', function() {
			it('should create instance', function() {
				new gdal.Feature(lyr);
			});
			it('instance should use fields from LayerDefn', function() {
				var feature = new gdal.Feature(lyr);
				assert.deepEqual(feature.fields.getNames(), lyr.fields.getNames());
			});
			it('should throw error if layer is destroyed', function() {
				var ds  = gdal.open('', 'w', 'Memory');
				var lyr = ds.layers.create('', null, gdal.Point);
				lyr.fields.add(fields);
				ds.close();
				assert.throws(function() {
					new gdal.Feature(lyr);
				});
			});
			it('should not throw error if layer is destroyed after feature is created', function() {
				var ds  = gdal.open('', 'w', 'Memory');
				var lyr = ds.layers.create('', null, gdal.Point);
				lyr.fields.add(fields);
				var feature = new gdal.Feature(lyr);
				ds.close();
				assert.doesNotThrow(function() {
					feature.defn.fields.getNames();
				});
			});
		});
		describe('w/FeatureDefn', function() {
			it('should create instance', function() {
				new gdal.Feature(defn);
			});
			it('instance should use fields from FeatureDefn', function() {
				var feature = new gdal.Feature(defn);
				assert.deepEqual(feature.fields.getNames(), defn.fields.getNames());
			});
		});
	});

	describe('instance', function() {
		describe('"fid" property', function() {
			describe('getter', function() {
				it('should return integer', function() {
					var feature = new gdal.Feature(defn);
					assert.equal(feature.fid, -1);
				});
			});
			describe('setter', function() {
				it('should set fid', function() {
					var feature = new gdal.Feature(defn);
					feature.fid = 5;
					assert.equal(feature.fid, 5);
				});
			});
		});

		describe('"defn" property', function() {
			describe('getter', function() {
				it('should return FeatureDefn', function() {
					var feature = new gdal.Feature(defn);
					assert.instanceOf(feature.defn, gdal.FeatureDefn);
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					var feature = new gdal.Feature(defn);
					assert.throws(function() {
						feature.defn = null;
					});
				});
			});
		});

		describe('"fields" property', function() {
			describe('getter', function() {
				it('should return FeatureFields', function() {
					var feature = new gdal.Feature(defn);
					assert.instanceOf(feature.fields, gdal.FeatureFields);
				});
			});
			describe('setter', function() {
				it('should throw error', function() {
					var feature = new gdal.Feature(defn);
					assert.throws(function() {
						feature.fields = null;
					}, /fields is a read-only property/);
				});
			});
			describe('count()', function() {
				it('should return an integer', function() {
					var feature = new gdal.Feature(defn);
					assert.equal(feature.fields.count(), 3);
				});
			});
			describe('set()', function() {
				describe('w/id argument', function() {
					it('should properly set values', function() {
						var feature = new gdal.Feature(defn);

						feature.fields.set(0, 5);
						feature.fields.set(1, 'test');
						feature.fields.set(2, 3.14);

						assert.equal(feature.fields.get(0), 5);
						assert.equal(feature.fields.get(1), 'test');
						assert.closeTo(feature.fields.get(2), 3.14, 0.0001);
					});
					it('should unset field if value is null', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set(1, 'test');
						feature.fields.set(1, null);
						assert.isNull(feature.fields.get(1));
					});
					it('should throw an error if id is out of range', function() {
						var feature = new gdal.Feature(defn);
						assert.throws(function() {
							feature.fields.set(100, 'test');
						});
					});
				});
				describe('w/name argument', function() {
					it('should properly set values', function() {
						var feature = new gdal.Feature(defn);

						feature.fields.set('id', 5);
						feature.fields.set('name', 'test');
						feature.fields.set('value', 3.14);

						assert.equal(feature.fields.get('id'), 5);
						assert.equal(feature.fields.get('name'), 'test');
						assert.closeTo(feature.fields.get('value'), 3.14, 0.0001);
					});
					it('should unset field if value is null', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set('name', 'test');
						feature.fields.set('name', null);
						assert.isNull(feature.fields.get('name'));
					});
					it('should throw an error if field name does not exist', function() {
						var feature = new gdal.Feature(defn);
						assert.throws(function() {
							feature.fields.set('bogus', 'test');
						});
					});
				});
				describe('w/array argument', function() {
					it('should properly set all fields', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set([5, 'test', 3.14]);
						assert.equal(feature.fields.get(0), 5);
						assert.equal(feature.fields.get(1), 'test');
						assert.closeTo(feature.fields.get(2), 3.14, 0.0001);
					});
				});
				describe('w/object argument', function() {
					it('should properly set all fields', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set({id:5, name:'test', value:3.14});
						assert.equal(feature.fields.get(0), 5);
						assert.equal(feature.fields.get(1), 'test');
						assert.closeTo(feature.fields.get(2), 3.14, 0.0001);
					});
				});
			});
			describe('get()', function() {
				describe('w/id argument', function() {
					it('should properly get values', function() {
						var feature = new gdal.Feature(defn);

						feature.fields.set(0, 5);
						feature.fields.set(1, 'test');
						feature.fields.set(2, 3.14);

						assert.equal(feature.fields.get(0), 5);
						assert.equal(feature.fields.get(1), 'test');
						assert.closeTo(feature.fields.get(2), 3.14, 0.0001);
					});
					it('should return unset fields as null', function() {
						var feature = new gdal.Feature(defn);

						assert.isNull(feature.fields.get(0));
						assert.isNull(feature.fields.get(1));
						assert.isNull(feature.fields.get(2));
					});
					it('should throw an error if id out of range', function() {
						var feature = new gdal.Feature(defn);
						assert.throws(function() {
							feature.fields.get(100);
						});
					});
				});
				describe('w/name argument', function() {
					it('should properly get/set values', function() {
						var feature = new gdal.Feature(defn);

						feature.fields.set('id', 5);
						feature.fields.set('name', 'test');
						feature.fields.set('value', 3.14);

						assert.equal(feature.fields.get('id'), 5);
						assert.equal(feature.fields.get('name'), 'test');
						assert.closeTo(feature.fields.get('value'), 3.14, 0.0001);
					});
					it('should return unset fields as null', function() {
						var feature = new gdal.Feature(defn);

						assert.isNull(feature.fields.get('id'));
						assert.isNull(feature.fields.get('name'));
						assert.isNull(feature.fields.get('value'));
					});
					it('should throw an error if field name doesnt exist', function() {
						var feature = new gdal.Feature(defn);
						assert.throws(function() {
							feature.fields.get('bogus');
						});
					});
				});
			});
			describe('toObject()', function() {
				it('should return the fields as a JSON object', function() {
					var feature = new gdal.Feature(defn);
					feature.fields.set([5, 'test', 3.14]);
					var obj = feature.fields.toObject();
					assert.equal(obj.id, 5);
					assert.equal(obj.name, 'test');
					assert.closeTo(obj.value, 3.14, 0.0001);
				});
			});
			describe('toJSON()', function() {
				it('should return the fields as a stringified JSON object', function() {
					var feature = new gdal.Feature(defn);
					feature.fields.set([5, 'test', 3.14]);
					var obj = JSON.parse(feature.fields.toJSON());
					assert.equal(obj.id, 5);
					assert.equal(obj.name, 'test');
					assert.closeTo(obj.value, 3.14, 0.0001);
				});
			});
			describe('toArray()', function() {
				it('should return an array of field values', function() {
					var feature = new gdal.Feature(defn);
					feature.fields.set([5, 'test', 3.14]);
					var array = feature.fields.toArray();
					assert.equal(array[0], 5);
					assert.equal(array[1], 'test');
					assert.closeTo(array[2], 3.14, 0.0001);
				});
			});
			describe('forEach()', function() {
				it('should return pass each value and key to callback', function() {
					var feature = new gdal.Feature(defn);
					var expected_keys   = ['id', 'name', 'value'];
					var expected_values = [5, 'test', 3.14];
					var values = [];
					var keys = [];
					feature.fields.set(expected_values);
					feature.fields.forEach(function(value, key) {
						values.push(value);
						keys.push(key);
					});
					assert.equal(keys.length, 3);
					assert.deepEqual(keys, expected_keys);
					assert.equal(values[0], expected_values[0]);
					assert.equal(values[1], expected_values[1]);
					assert.closeTo(values[2], expected_values[2], 0.0001);
				});
			});
			describe('getNames()', function() {
				it('should return an array of field names', function() {
					var feature = new gdal.Feature(defn);
					var expected_names = ['id', 'name', 'value'];
					assert.deepEqual(feature.fields.getNames(), expected_names);
				});
			});
			describe('indexOf()', function() {
				it('should return index of field name', function() {
					var feature = new gdal.Feature(defn);
					assert.equal(feature.fields.indexOf('name'), 1);
				});
			});
			describe('reset()', function() {
				describe('w/no argument', function() {
					it('should reset all fields to null', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set([5, 'test', 3.14]);
						feature.fields.reset();
						assert.isNull(feature.fields.get(0));
						assert.isNull(feature.fields.get(1));
						assert.isNull(feature.fields.get(2));
					});
				});
				describe('w/object argument', function() {
					it('should set fields from object and reset others', function() {
						var feature = new gdal.Feature(defn);
						feature.fields.set([5, 'test', 3.14]);
						feature.fields.reset({name: 'reset'});
						assert.isNull(feature.fields.get(0));
						assert.equal(feature.fields.get(1), 'reset');
						assert.isNull(feature.fields.get(2));
					});
				});
			});
		});

		describe('clone()', function() {
			it('should return new Feature', function() {
				var feature = new gdal.Feature(defn);
				var clone = feature.clone();
				assert.instanceOf(clone, gdal.Feature);
				assert.notEqual(clone, feature);
			});
		});
		describe('setGeometry()', function() {
			it('should set geometry', function() {
				var feature = new gdal.Feature(defn);
				feature.setGeometry(new gdal.Point(5, 10));
				var pt = feature.getGeometry();
				assert.equal(pt.x, 5);
				assert.equal(pt.y, 10);
			});
			it('should clear geometry if null is passed', function() {
				var feature = new gdal.Feature(defn);
				feature.setGeometry(new gdal.Point(5, 10));
				feature.setGeometry(null);
				assert.isNull(feature.getGeometry());
			});
			it('should clear geometry if undefined is passed', function() {
				var feature = new gdal.Feature(defn);
				feature.setGeometry(new gdal.Point(5, 10));
				feature.setGeometry(undefined);
				assert.isNull(feature.getGeometry());
			});
			/*
			// http://gdal.org/1.11/ogr/classOGRFeature.html#af1181ade837a52129ea25b46dd50cf30
			// "checking for geometry type not yet implemented"
			it('should throw error if wrong geometry type', function(){
				var feature = new gdal.Feature(defn);
				assert.throws(function(){
					feature.setGeometry(new gdal.LineString());
				});
			});
			*/
		});
		describe('getGeometry()', function() {
			it('should get geometry', function() {
				var feature = new gdal.Feature(defn);
				feature.setGeometry(new gdal.Point(5, 10));
				var pt = feature.getGeometry();
				assert.equal(pt.x, 5);
				assert.equal(pt.y, 10);
			});
			it('should return null if geometry is not set', function() {
				var feature = new gdal.Feature(defn);
				var geom = feature.getGeometry();
				assert.isNull(geom);
			});
		});
		describe('setFrom()', function() {
			it('should set fields and geometry from other feature', function() {
				var feature1 = new gdal.Feature(defn);
				var feature2 = new gdal.Feature(defn);
				feature1.setGeometry(new gdal.Point(5, 10));
				feature1.fields.set([5, 'test', 3.14]);
				feature2.setFrom(feature1);

				var pt = feature2.getGeometry();
				assert.equal(pt.x, 5);
				assert.equal(pt.y, 10);
				assert.equal(feature2.fields.get(0), 5);
				assert.equal(feature2.fields.get(1), 'test');
				assert.closeTo(feature2.fields.get(2), 3.14, 0.0001);
			});
		});
	});
});
