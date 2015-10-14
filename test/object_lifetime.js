'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;
var fs = require('fs');
var path = require('path');

describe('object cache', function() {
	it('should return same object if pointer is same', function(){
		for(var i = 0; i<10; i++) {
			gdal.log("Object Cache test run #"+i);
			var ds = gdal.open('temp', 'w', 'MEM', 4, 4, 1);
			var band1 = ds.bands.get(1);
			var band2 = ds.bands.get(1);
			assert.instanceOf(band1, gdal.RasterBand);
			assert.equal(band1, band2);
			assert.equal(band1.size.x, 4);
			assert.equal(band2.size.x, 4);
			gc();
		}
	});
});
describe('object lifetimes', function() {
	it('datasets should stay alive until all bands go out of scope', function(){
		var ds = gdal.open('temp', 'w', 'MEM', 4, 4, 1);
		var band = ds.bands.get(1);

		var ds_uid = ds._uid;
		var band_uid = band._uid;
		
		ds = null;
		gc();

		assert.isTrue(gdal._isAlive(ds_uid));
		assert.isTrue(gdal._isAlive(band_uid));

		band = null;
		gc();

		assert.isFalse(gdal._isAlive(ds_uid));
		assert.isFalse(gdal._isAlive(band_uid));
	});	
	it('bands should immediately be garbage collected as they go out of scope', function(){
		var ds = gdal.open('temp', 'w', 'MEM', 4, 4, 1);
		var band = ds.bands.get(1);

		var ds_uid = ds._uid;
		var band_uid = band._uid;
		
		band = null;
		gc();

		assert.isTrue(gdal._isAlive(ds_uid));
		assert.isFalse(gdal._isAlive(band_uid));
	});
	it('datasets should stay alive until all layers go out of scope', function(){
		var ds = gdal.open(path.join(__dirname, "data/shp/sample.shp"));
		var layer = ds.layers.get(0);

		var ds_uid = ds._uid;
		var layer_uid = layer._uid;
		
		ds = null;
		gc();

		assert.isTrue(gdal._isAlive(ds_uid));
		assert.isTrue(gdal._isAlive(layer_uid));

		layer = null;
		gc();

		assert.isFalse(gdal._isAlive(ds_uid));
		assert.isFalse(gdal._isAlive(layer_uid));
	});	
	it('layers should immediately be garbage collected as they go out of scope', function(){
		var ds = gdal.open(path.join(__dirname, "data/shp/sample.shp"));
		var layer = ds.layers.get(0);

		var ds_uid = ds._uid;
		var layer_uid = layer._uid;
		
		layer = null;
		gc();

		assert.isTrue(gdal._isAlive(ds_uid));
		assert.isFalse(gdal._isAlive(layer_uid));
	});
});