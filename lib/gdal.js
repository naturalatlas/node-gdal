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
gdal.GDALDrivers.prototype.forEach = defaultForEach;

//add 'w' mode to gdal.open() method and also GDAL2-style driver selection 
var open = gdal.open;
gdal.open = function(filename, mode, drivers, x_size, y_size, n_bands, datatype, options){
	if (typeof drivers === 'string') {
		drivers = [drivers];
	}

	if (mode == 'w') {
		//create file with given driver
		if(drivers.length != 1){
			throw new Error("Only one driver can be used to create a file");
		}
		var driver = gdal.drivers.get(drivers[0]);
		var args = Array.prototype.slice.call(arguments, 3);
		args.unshift(filename);
	 	return driver.create.apply(driver, args);
	} else {
		if (arguments.length > 2) {
			// open file with driver list
			// loop through given drivers trying to open file 
			var ds;
			drivers.forEach(function(driver_name, i){
				var driver = gdal.drivers.get(driver_name);
				try {
					ds = driver.open(filename, mode);
					return false;
				} catch (err) {
					
				}
			});

			if(!ds) throw new Error("Error opening dataset");
			return ds;

		} else {
			//call gdal.open() method normally
			return open.apply(gdal, arguments);
		}
	}
}

function fieldTypeFromValue(val) {
	var type = typeof val;
	if(type == 'number') {
		if(val % 1 === 0) return gdal.OFTInteger;
		else return gdal.OFTReal;
	} else if (type == 'string') {
		return gdal.OFTString;
	} else if (type == 'boolean') {
		return gdal.OFTInteger;
	} else if (val instanceof Date) {
		return gdal.OFTDateTime;
	} else if (val instanceof Array) {
		var sub_type = fieldTypeFromValue(val[0]);
		switch (sub_type) {
			case gdal.OFTString : return gdal.OFTStringList;
			case gdal.OFTInteger : return gdal.OFTIntegerList;
			case gdal.OFTReal : return gdal.OFTRealList;
			default : throw new Error("Array element cannot be converted into OGRFieldType");
		}
	} else if (val instanceof Buffer) {
		return gdal.OFTBinary;
	} else {
		throw new Error("Value cannot be converted into OGRFieldType");
	}
}

gdal.LayerFields.prototype.fromJSON = function(obj, approx_ok) {
	if(arguments.length < 2) {
		approx_ok = false;
	}
	var field_names = Object.keys(obj);
	for (var i = 0; i < field_names.length; i++){	
		var name  = field_names[i];
		var value = obj[field_names[i]];
		var type  = fieldTypeFromValue(value);
		var def   = new gdal.FieldDefn(name, type);
		this.add(def, approx_ok);
	}
}