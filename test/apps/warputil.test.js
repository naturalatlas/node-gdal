'use strict';

var WarpUtil = require('../../lib/gdal').WarpUtil;
var assert = require('chai').assert;
var fs = require('fs');
var path = require('path');

describe('warputil', function(){
	it('warp', function(done){
		var warper = new WarpUtil();

		var warpOptions = {
			source: path.resolve('./test/data/BlueMarble.TIFF'),
			target: path.resolve('./test/data/temp/WarpOut.tif'),
			width: 512,
			height: 512,
			targetSrs: 'EPSG:4326',
			format: 'GTiff',
			extents: {
				minX: -100,
				minY: 10,
				maxX: -70,
				maxY: 40
			},
			silent: true
		};
		var ret = warper.warp(warpOptions);
		assert.equal(ret, 0);
		fs.stat(warpOptions.target, function(error, stat){
			assert.equal(error, null);
			assert(stat.isFile());
			done();
		});
	});
});