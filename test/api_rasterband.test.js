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
		describe('"pixels" property', function() {
			describe('getter', function() {
				it('should return pixel collection', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.instanceOf(band.pixels, gdal.RasterBandPixels);
				});
			});
			describe('setter', function() {
				it('should throw an error', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.pixels = null;
					});
				});
			});
			describe('get()', function() {
				it('should return a number', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.equal(band.pixels.get(200, 300), 10);
				});
				it('should throw an error if x,y is out of bounds', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.pixels.get(-1, -1);
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.pixels.get(200, 300);
					});
				});
			});
			describe('set()', function() {
				it('should set the pixel to the value', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					band.pixels.set(10, 20, 30);
					assert.equal(band.pixels.get(10, 20), 30);
					band.pixels.set(10, 20, 33.6);
					assert.equal(band.pixels.get(10, 20), 34);
				});
				it('should throw an error if x,y is out of bounds', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.pixels.set(-1, -1, 20);
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.pixels.set(10, 20, 30);
					});
				});
			});
			describe('read()', function() {
				it('should return a TypedArray', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					var w = 20;
					var h = 30;
					var data = band.pixels.read(190, 290, w, h);
					assert.instanceOf(data, Uint8Array);
					assert.equal(data.length, w*h);
					assert.equal(data[10*20+10], 10);
				});
				describe('w/data argument', function(){					
					it('should put the data in the existing array', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Uint8Array(20*30);
						data[15] = 31;
						var result = band.pixels.read(0,0,20,30,data);
						assert.equal(data, result);
						assert.equal(data[15], 0);
					});
					it('should create new array if null', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = band.pixels.read(0,0,20,30,null);
						assert.instanceOf(data, Uint8Array);
						assert.equal(data.length, 20*30);
					});
					it('should throw error if array is too small', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Uint8Array(20*30);
						assert.throws(function(){ 
							band.pixels.read(0,0,20,31,data);
						});
					});
					it('should automatically translate data to array data type', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Float64Array(20*30);
						band.pixels.read(0,0,20,30,data);
					});
				});
				describe('w/buffer_width,buffer_height arguments', function(){
					it('should create new array with given dimensions if array isn\'t given', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = band.pixels.read(0,0,20,30,null,10,15);
						assert.equal(data.length, 10*15);
					});
					it('should throw error if given array is smaller than given dimensions', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Float64Array(10,14);
						assert.throws(function(){
							band.pixels.read(0,0,20,30,data,10,15);
						});
					});
				});
				describe('w/type argument', function(){
					it('should be ignored if typed array is given', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Float64Array(20*30);
						var result = band.pixels.read(0,0,20,30,data,20,30,gdal.GDT_Byte);
						assert.instanceOf(result, Float64Array);
					});
					it('should create output array with given type', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = band.pixels.read(0,0,20,30,null,20,30,gdal.GDT_Float64);
						assert.instanceOf(data, Float64Array);
					});
				});
				describe('w/pixel_space,line_space arguments', function(){
					it('should read data with space between values', function(){
						var w = 16, h = 16;
						var ds   = gdal.open('temp', 'w', 'MEM', w, h, 2, gdal.GDT_Byte);
						var red  = ds.bands.get(1);
						var blue = ds.bands.get(2);
						red.fill(1);
						blue.fill(2);

						var interleaved = new Uint8Array(w*h*2);
						red.pixels.read(0,0,w,h,interleaved,w,h,gdal.GDT_Byte,2,2*w);
						blue.pixels.read(0,0,w,h,interleaved.subarray(1),w,h,gdal.GDT_Byte,2,2*w);
  
						for(var y = 0; y < h; y++) {
							for(var x = 0; x < w; x++) {
								var r = interleaved[x*2+0+y*w*2];
								var b = interleaved[x*2+1+y*w*2];
								assert.equal(r, 1);
								assert.equal(b, 2);
							}
						}
					});
					it('should throw error if array is not long enough to store result', function(){
						var w = 16, h = 16;
						var ds   = gdal.open('temp', 'w', 'MEM', w, h, 2, gdal.GDT_Byte);
						var red  = ds.bands.get(1);
						var blue = ds.bands.get(2);
						red.fill(1);
						blue.fill(2);

						var interleaved = new Uint8Array(w*h*2);
						red.pixels.read(0,0,w,h,interleaved,w,h,gdal.GDT_Byte,2,2*w);
						assert.throws(function(){
							blue.pixels.read(0,0,w,h,interleaved.subarray(2),w,h,gdal.GDT_Byte,2,2*w);
						})
					});
				});
				it('should throw an error if region is out of bounds', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					assert.throws(function(){
						band.pixels.read(20, 20, 16, 16);
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.pixels.read(0,0,16,16);
					});
				});
			});
			describe('write()', function() {
				it('should write data from TypedArray', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					var w = 16, h = 16;

					var data = new Uint8Array(w*h);
					for(var i = 0; i < w*h; i++) data[i] = i;

					band.pixels.write(100, 120, w, h, data);

					var result = band.pixels.read(100, 120, w, h, data);
					for(var i = 0; i < w*h; i++) {
						assert.equal(result[i], data[i]);
					}
				});
				it('should throw error if array is too small', function(){
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					var w = 16, h = 16;
					var data = new Uint8Array(w*h-1);

					assert.throws(function(){ 
						band.pixels.write(100, 120, w, h, data);
					});
				});
				it('should automatically translate data to array data type', function(){
					var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					var w = 16, h = 16;

					var data = new Float64Array(w*h);
					for(var i = 0; i < w*h; i++) data[i] = i+0.33;

					band.pixels.write(100, 120, w, h, data);

					var result = band.pixels.read(100, 120, w, h, data);
					for(var i = 0; i < w*h; i++) {
						assert.equal(result[i], Math.floor(data[i]));
					}
				});
				describe('w/buffer_width,buffer_height arguments', function(){
					it('should throw error if given array is smaller than given dimensions', function(){
						var ds   = gdal.open('temp', 'w', 'MEM', 256, 256, 1, gdal.GDT_Byte);
						var band = ds.bands.get(1);
						var data = new Float64Array(10,14);
						assert.throws(function(){
							band.pixels.write(0,0,20,30,data,10,15);
						});
					});
				});
				describe('w/pixel_space,line_space arguments', function(){
					it('should skip spaces in given data', function(){
						var w = 16, h = 16;
						var ds   = gdal.open('temp', 'w', 'MEM', w, h, 2, gdal.GDT_Byte);
						var red  = ds.bands.get(1);
						var blue = ds.bands.get(2);

						var interleaved = new Uint8Array(w*h*2);
						for(var y = 0; y < h; y++) {
							for(var x = 0; x < w; x++) {
								interleaved[x*2+0+y*w*2] = 1;
								interleaved[x*2+1+y*w*2] = 2;
							}
						}

						red.pixels.write(0,0,w,h,interleaved,w,h,2,2*w);
						blue.pixels.write(0,0,w,h,interleaved.subarray(1),w,h,2,2*w);


  						var data;
  						data = red.pixels.read(0,0,w,h);
  						for(var i = 0; i < data.length; i++) assert.equal(data[i], 1);
  						data = blue.pixels.read(0,0,w,h);
  						for(var i = 0; i < data.length; i++) assert.equal(data[i], 2);

					});
					it('should throw error if array is not long enough', function(){
						var w = 16, h = 16;
						var ds   = gdal.open('temp', 'w', 'MEM', w, h, 2, gdal.GDT_Byte);
						var red  = ds.bands.get(1);
						var blue = ds.bands.get(2);

						var interleaved = new Uint8Array(w*h*2);
						
						red.pixels.write(0,0,w,h,interleaved,w,h,2,2*w);
						assert.throws(function(){
							blue.pixels.write(0,0,w,h,interleaved.subarray(2),w,h,2,2*w);
						})
					});
				});
				it('should throw an error if region is out of bounds', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					var data = new Uint8Array(16*16);
					assert.throws(function(){
						band.pixels.write(20, 20, 16, 16, data);
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);
					ds.close();
					var data = new Uint8Array(16*16);
					assert.throws(function(){
						band.pixels.write(0,0,16,16, data);
					});
				});
			});
			describe('readBlock()', function() {
				it('should return TypedArray', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);

					var data = band.pixels.readBlock(0, 0);
					assert.instanceOf(data, Uint8Array);
					assert.equal(data.length, band.blockSize.x * band.blockSize.y)
				});
				it('should throw error if offsets are out of range', function(){
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					assert.throws(function(){ 
						band.pixels.readBlock(-1, 0);
					});
				});
				describe('w/ data argument', function(){
					it('should read data into existing', function(){
						var ds   = gdal.open(__dirname + '/data/sample.tif');
						var band = ds.bands.get(1);
						var data = new Uint8Array(band.blockSize.x*band.blockSize.y);
						var result = band.pixels.readBlock(0,0,data);
						assert.equal(result, data);
					})
					it('should throw error if given array is not big enough', function(){
						var ds   = gdal.open(__dirname + '/data/sample.tif');
						var band = ds.bands.get(1);
						var data = new Uint8Array(band.blockSize.x*band.blockSize.y-1);
						assert.throws(function(){
							band.pixels.readBlock(0,0,data);
						});
					});
					it('should throw error if given array is not the right type', function(){
						var ds   = gdal.open(__dirname + '/data/sample.tif');
						var band = ds.bands.get(1);
						var data = new Float64Array(band.blockSize.x*band.blockSize.y);
						assert.throws(function(){
							band.pixels.readBlock(0,0,data);
						});
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);
					ds.close();
					assert.throws(function(){
						band.pixels.readBlock(0, 0);
					});
				});
			});
			describe('writeBlock()', function() {
				it('should write data from TypedArray', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);

					var length = band.blockSize.x*band.blockSize.y;
					var data = new Uint8Array(length);
					for(var i = 0; i < length; i++) data[i] = i;

					band.pixels.writeBlock(0,0,data);

					var result = band.pixels.readBlock(0,0);

					for(var i = 0; i < length; i++){
						assert.equal(result[i], data[i])
					}
				});
				it('should throw error if offsets are out of range', function(){
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);

					var length = band.blockSize.x*band.blockSize.y;
					var data = new Uint8Array(length);

					assert.throws(function(){ 
						band.pixels.writeBlock(0, 100, data);
					});
				});
				it('should throw error if given array is not big enough', function(){
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);

					var length = band.blockSize.x*band.blockSize.y-1;
					var data = new Uint8Array(length);

					assert.throws(function(){ 
						band.pixels.writeBlock(0, 0, data);
					});
				});
				it('should throw error if given array is not the right type', function(){
					var ds   = gdal.open(__dirname + '/data/sample.tif');
					var band = ds.bands.get(1);

					var length = band.blockSize.x*band.blockSize.y;
					var data = new Float64Array(length);

					assert.throws(function(){
						band.pixels.writeBlock(0, 0, data);
					});
				});
				it('should throw error if dataset already closed', function() {
					var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
					var band = ds.bands.get(1);

					var length = band.blockSize.x*band.blockSize.y;
					var data = new Uint8Array(length);
					ds.close();
					assert.throws(function(){
						band.pixels.writeBlock(0, 0, data);
					});
				});
			});
		});
		describe("fill()", function(){
			it("should set all pixels to given value", function(){
				var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
				var band = ds.bands.get(1);
				band.fill(5);
				var data = band.pixels.read(0,0,16,16);
				for(var i = 0; i < data.length; i++) {
					assert.equal(data[i], 5);
				}
			});
			it('should throw error if dataset already closed', function() {
				var ds   = gdal.open('temp', 'w', 'MEM', 16, 16, 1, gdal.GDT_Byte);
				var band = ds.bands.get(1);
				ds.close();
				assert.throws(function(){
					band.fill(5);
				});
			});
		});
	});
});