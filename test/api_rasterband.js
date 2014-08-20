'use strict';

var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');

describe('gdal.RasterBand', function() {
	afterEach(gc);

	it('should not be instantiable', function() {
		assert.throws(function(){
			new gdal.RasterBand();
		});
	});
	describe('instance', function() {
		describe('"ds" property', function() {
			describe('getter', function() {
				it('should return gdal.Dataset', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.equal(band.ds, ds);
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);					
					assert.throws(function(){
						band.ds = null;
					});
				});
			});
		});
		describe('"colorInterpretation" property', function() {
			describe('getter', function() {
				it('should return colorInterpretation', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.equal(band.colorInterpretation, gdal.GCI_GrayIndex);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.colorInterpretation);
					});
				});
			});
			describe('setter', function() {
				it('should set colorInterpretation', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.colorInterpretation = gdal.GCI_RedBand;
					assert.equal(band.colorInterpretation, gdal.GCI_RedBand);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.colorInterpretation = gdal.GCI_RedBand;
					});
				});
			});
		});
		describe('"description" property', function() {
			describe('getter', function() {
				it('should return string', function() {
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					assert.equal(band.description, 'hshade17');
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.description);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.description = "test";
					});
				});
			});
		});
		describe('"id" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.equal(band.id, 1);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.id);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.id = 5;
					});
				});
			});
		});
		describe('"size" property', function() {
			describe('getter', function() {
				it('should return object', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 128, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.deepEqual(band.size, {x: 128, y: 256});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.size);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.size = {x:128, y:128};
					});
				});
			});
		});
		describe('"blockSize" property', function() {
			describe('getter', function() {
				it('should return object', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.deepEqual(band.blockSize, {x: 984, y: 8});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.blockSize);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.blockSize = {x:128, y:128};
					});
				});
			});
		});
		describe('"unitType" property', function() {
			describe('getter', function() {
				it('should return string', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.isString(band.unitType);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.unitType);
					});
				});
			});
			describe('setter', function() {
				it('should set unitType', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.unitType = 'm';
					assert.equal(band.unitType, 'm');
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.unitType = 'm';
					});
				});
			});
		});
		describe('"dataType" property', function() {
			describe('getter', function() {
				it('should return dataType', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Float64);
					var band = ds.bands.get(1);
					assert.equal(band.dataType, gdal.GDT_Float64);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.dataType);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.dataType = gdal.GDT_Float64;
					});
				});
			});
		});
		describe('"readOnly" property', function() {
			describe('getter', function() {
				it('should return true on readOnly dataset', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.isTrue(band.readOnly);
				});
				it('should return false on writable dataset', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.isFalse(band.readOnly);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.readOnly);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.readOnly = true;
					});
				});
			});
		});
		describe('"minimum" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					assert.equal(band.minimum, 177);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.minimum);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.minimum = 5;
					});
				});
			});
		});
		describe('"maximum" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					assert.equal(band.maximum, 182);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.maximum);
					});
				});
			});
			describe('setter', function() {
				it('should throw error', function() {					
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.maximum = 5;
					});
				});
			});
		});
		describe('"offset" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.equal(band.offset, 0);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.offset);
					});
				});
			});
			describe('setter', function() {
				it('should set offset', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.offset = 16;
					assert.equal(band.offset, 16);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.offset = 16;
					});
				});
			});
		});
		describe('"scale" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.equal(band.scale, 1);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.scale);
					});
				});
			});
			describe('setter', function() {
				it('should set scale', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.scale = 2;
					assert.equal(band.scale, 2);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.scale = 2;
					});
				});
			});
		});
		describe('"noDataValue" property', function() {
			describe('getter', function() {
				it('should return number', function() {
					var ds   = gdal.open(__dirname + '/data/dem_azimuth50_pa.img');
					var band = ds.bands.get(1);
					assert.equal(band.noDataValue, 0);
				});
				it('should return null if not set', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.isNull(band.noDataValue);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						console.log(band.noDataValue);
					});
				});
			});
			describe('setter', function() {
				it('should set noDataValue', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.noDataValue = 5;
					assert.equal(band.noDataValue, 5);
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.noDataValue = 5;
					});
				});
			});
		});
	});
});