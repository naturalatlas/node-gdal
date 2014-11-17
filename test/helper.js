var gdal = require('../lib/gdal.js');

beforeEach(function(){
	gdal.log("BEGIN TEST: "+this.currentTest.title);
});

afterEach(function(){
	gdal.log("END TEST: "+this.currentTest.title);
});