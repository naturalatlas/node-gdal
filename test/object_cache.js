'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;
var fs = require('fs');

describe('object cache', function() {
	it('should return same object if pointer is same', function(){
		for(var i = 0; i<10; i++) {
			gdal.log("Object Cache test run #"+i);
			var ds = gdal.open('temp', 'w', 'MEM', 4, 4, 1);
			var band1 = ds.bands.get(1);
			//var band2 = ds.bands.get(1);
			//assert.instanceOf(band1, gdal.RasterBand);
			//assert.equal(band1, band2);
			//assert.equal(band1.size.x, 4);
			//assert.equal(band2.size.x, 4);
			gc();
		}
	});
});