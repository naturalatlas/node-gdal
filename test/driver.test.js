'use strict';

var test = require('tap').test;
var gdal = require('../');

test('Driver', function(t) {
    var expected = { 
        "GTiff": {
            DMD_LONGNAME: "GeoTIFF",
            DMD_MIMETYPE: "image/tiff",
            DMD_EXTENSION: "tif",
            DCAP_CREATE: 'YES'
        },
        "VRT": {
            DMD_LONGNAME: "Virtual Raster",
            DMD_MIMETYPE: undefined,
            DMD_EXTENSION: "vrt",
            DCAP_CREATE: 'YES'
        },
        "MEM": {
            DMD_LONGNAME: "In Memory Raster",
            DMD_MIMETYPE: undefined,
            DMD_EXTENSION: undefined,
            DCAP_CREATE: 'YES'
        }
    };
    Object.keys(expected).forEach(function(o) {
        var driver = gdal.getDriverByName(o);
        var metadata = driver.getMetadata();
        var expected_meta = expected[o];
        t.equal(expected_meta.DMD_LONGNAME,metadata.DMD_LONGNAME);
        t.equal(expected_meta.DMD_MIMETYPE,metadata.DMD_MIMETYPE);
        t.equal(expected_meta.DMD_EXTENSION,metadata.DMD_EXTENSION);
    });
    t.end();
});
