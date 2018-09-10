var assert = require('chai').assert;
var gdal = require('../lib/gdal.js');
var fileUtils = require('./utils/file.js');

describe('gdal.Dataset', function() {
	afterEach(gc);
	describe('instance', function() {
		describe('read()', function() {
			var w=32;
			var h=32;
			var bufw=32;
			var bufh=32;
			var bandCount=3;
			var pixelSpace=0;
			var lineSpace=0;
			var bandSpace=0;
			
			it('should read data (destination buffer pixel interleaved)', function() {
				bandSpace=1; 
				pixelSpace=4;
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var data = ds.read(
				  0,0,w,h,null,bufw,bufh,
				  gdal.GDT_Byte,bandCount,null,pixelSpace,lineSpace,bandSpace,null);
				ds.close();
			});
			it('should read data (destination buffer band sequential)', function() {
				bandSpace=0;
				pixelSpace=0;
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var data = ds.read(
				  0,0,w,h,null,bufw,bufh,
				  gdal.GDT_Byte,bandCount,null,pixelSpace,lineSpace,bandSpace,null);
				ds.close();
			});
			it('should read data w/default parameters', function() {
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var data = ds.read(0,0,w,h);
				assert.instanceOf(data, Uint8Array);
				assert.equal(data.length, w * h * bandCount);
				ds.close();
			});
			it('should read data w/default parameters into passed array', function() {
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var data = new Uint8Array(new ArrayBuffer(w * h * bandCount));
				ds.read(0,0,w,h,data);
				ds.close();
			});
			it('should read data w/bandMap argument', function() {
				bandSpace=0;
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var bandMap = [1,2,3];
				var data = ds.read(
				  0,0,w,h,null,bufw,bufh,
				  gdal.GDT_Byte,bandCount,bandMap,pixelSpace,lineSpace,bandSpace,null);
				ds.close();
			});
	    });
		describe('write()', function() {
			
			it('should write data w/default parameters', function() {
				var ds = gdal.open(__dirname + '/data/multiband.tif');
				var w=ds.rasterSize.x;
				var h=ds.rasterSize.y;
				var bufw=w;
				var bufh=h;
				var bandCount=3;
				
				var ods  = gdal.open('temp', 'w', 'MEM', w, h, bandCount, gdal.GDT_Byte);
				
				var data = ds.read(0,0,w,h);
				
				ods.write(0,0,w,h,data);
				
				ods.close();
				ds.close();
				
				//console.dir({ds.bands});
				/* var data = ds.read(
				  0,0,w,h);
				assert.instanceOf(data, Uint8Array);
				assert.equal(data.length, w * h * bandCount);
				ds.close(); */
			});
	    });
	});
});