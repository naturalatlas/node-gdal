'use strict';

var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('assert');

describe('Open', function() {
	it('should throw when invalid file', function() {
		var filename = path.join(__dirname,"data/invalid");
		assert.throws(function() {
			gdal.open(filename);
		});
	});
});