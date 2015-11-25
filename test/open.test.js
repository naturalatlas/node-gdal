var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('assert');

describe('Open', function() {
	afterEach(gc);

	it('should throw when invalid file', function() {
		var filename = path.join(__dirname, 'data/invalid');
		assert.throws(function() {
			gdal.open(filename);
		}, /Error opening dataset/);
	});
});
