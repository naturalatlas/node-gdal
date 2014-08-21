var gdal = require('../lib/gdal.js');

// gc tracing
try {
	gdal.startLogging(__dirname + '/artifacts/log.txt');
} catch (e) {}

// seg fault handler
var SegfaultHandler;
try {
	SegfaultHandler = require('segfault-handler');
	SegfaultHandler.registerHandler();
} catch (err) {}