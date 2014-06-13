var path         = require('path');
var pkg          = require('../package.json');
var binding_path = path.join(__dirname, '../', pkg.binary.module_path);
var module_path  = path.join(binding_path, pkg.binary.module_name + '.node');

var gdal = module.exports = require(module_path);
var ogr  = gdal.ogr;

gdal.RasterBandCollection.prototype.forEach = function(iterator){
	var n = this.count();
	for(var i = 1; i <= n; i++){
		if (iterator(this.get(i)) === false) return;
	}
}

gdal.FieldCollection.prototype.forEach = function(iterator){
	var n = this.count();
	for(var i = 0; i < n; i++){
		if (iterator(this.get(i)) === false) return;
	}
}

gdal.LayerCollection.prototype.forEach = function(iterator){
	var n = this.count();
	for (var i = 0; i < n; i++) {
		if (iterator(this.get(i)) === false) return;
	}
}

gdal.FeatureCollection.prototype.forEach = function(iterator){
	var feature = this.first(); 
	while (feature) {
		if (iterator(feature) === false) return;
		this.next();
	}
}