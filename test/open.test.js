'use strict';

var test = require('tape').test;
var gdal = require('../');
var path = require('path');

test('Open Tiff', function(t) {
    var filename = path.join(__dirname,"data/sample.tif");
    var ds  = gdal.open(filename);
    t.equal(ds.getRasterXSize(),984);
    t.equal(ds.getRasterYSize(),804);
    t.equal(ds.getRasterCount(),1);
    var expected_geotransform = [
      -1134675.2952829634,
      7.502071930146189,
      0,
      2485710.4658232867,
      0,
      -7.502071930145942
    ];
    var actual_geotransform = ds.getGeoTransform();
    t.ok(Math.abs(actual_geotransform[0] - expected_geotransform[0]) < .00001);
    t.ok(Math.abs(actual_geotransform[1] - expected_geotransform[1]) < .00001);
    t.ok(Math.abs(actual_geotransform[2] - expected_geotransform[2]) < .00001);
    t.ok(Math.abs(actual_geotransform[3] - expected_geotransform[3]) < .00001);
    t.ok(Math.abs(actual_geotransform[4] - expected_geotransform[4]) < .00001);
    t.ok(Math.abs(actual_geotransform[5] - expected_geotransform[5]) < .00001);
    var srs = ds.getProjectionRef();
    t.ok(srs.indexOf('PROJCS') > -1);
    var band = ds.getRasterBand(1);
    var expected_stats = {
        min: 0,
        max: 100,
        mean: 29.725628716175223,
        std_dev: 36.98885954363488
    };
    var actual_stats = band.getStatistics(0,1);
    t.ok(Math.abs(expected_stats.min - actual_stats.min) < .00001);
    t.ok(Math.abs(expected_stats.max - actual_stats.max) < .00001);
    t.ok(Math.abs(expected_stats.mean - actual_stats.mean) < .00001);
    t.ok(Math.abs(expected_stats.std_dev - actual_stats.std_dev) < .00001);
    t.end();
});
