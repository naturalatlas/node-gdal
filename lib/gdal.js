var path         = require('path');
var pkg          = require('../package.json');
var binding_path = path.join(__dirname, '../', pkg.binary.module_path);
var module_path  = path.join(binding_path, pkg.binary.module_name + '.node');
var data_path    = path.resolve(__dirname, '../deps/libgdal/gdal/data');

var gdal = module.exports = require(module_path);
var ogr  = gdal.ogr;

gdal.quiet();
gdal.setConfigOption("GDAL_DATA", data_path);

function defaultForEach(iterator) {
	var n = this.count();
	for(var i = 0; i < n; i++){
		if (iterator(this.get(i)) === false) return;
	}
}


gdal.DatasetBands.prototype.forEach = function(iterator){
	var n = this.count();
	for(var i = 1; i <= n; i++){
		if (iterator(this.get(i)) === false) return;
	}
};

gdal.LayerFeatures.prototype.forEach = function(iterator){
	var feature = this.first(); 
	while (feature) {
		if (iterator(feature) === false) return;
		this.next();
	}
}

gdal.LayerFields.prototype.forEach = defaultForEach;
gdal.FeatureFields.prototype.forEach = defaultForEach;
gdal.DatasetLayers.prototype.forEach = defaultForEach;
gdal.FeatureDefnFields.prototype.forEach = defaultForEach;
gdal.PolygonRings.prototype.forEach = defaultForEach;
gdal.LineStringPoints.prototype.forEach = defaultForEach;
gdal.GeometryCollectionChildren.prototype.forEach = defaultForEach;
gdal.RasterBandOverviews.prototype.forEach = defaultForEach;
