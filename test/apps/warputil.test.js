'use strict';

var WarpUtil = require('../../lib/gdal').WarpUtil;
var assert = require('chai').assert;
var fs = require('fs');
var path = require('path');

describe('warputil', function(){
	it('warp', function(){
		var warper = new WarpUtil();
		console.log(warper);
	});
});