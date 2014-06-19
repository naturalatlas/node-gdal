'use strict';

var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('"version" property', function() {
	it('should exist', function() {
		assert.match(gdal.version, /^\d+\.\d+\.\d+$/);
	});
});