var path         = require('path');
var pkg          = require('../package.json');
var binding_path = path.join(__dirname, '../', pkg.binary.module_path);
var module_path  = path.join(binding_path, pkg.binary.module_name + '.node');
var data_path    = path.resolve(__dirname, '../deps/libgdal/gdal/data');

var gdal = module.exports = require(module_path);

gdal.Point.Multi      = gdal.MultiPoint;
gdal.LineString.Multi = gdal.MultiLineString;
gdal.LinearRing.Multi = gdal.MultiLineString;
gdal.Polygon.Multi    = gdal.MultiPolygon;

gdal.quiet();
gdal.config = {
	get: gdal.getConfigOption,
	set: gdal.setConfigOption
};

delete gdal.getConfigOption;
delete gdal.setConfigOption;

gdal.config.set('GDAL_DATA', data_path);


// --- add full OGREnvelope functionality ---

gdal.Envelope = require('./envelope.js');
gdal.Envelope3D = require('./envelope_3d.js');

var getEnvelope = gdal.Geometry.prototype.getEnvelope;
gdal.Geometry.prototype.getEnvelope = function() {
	var obj = getEnvelope.apply(this, arguments);
	return new gdal.Envelope(obj);
};

var getEnvelope3D = gdal.Geometry.prototype.getEnvelope3D;
gdal.Geometry.prototype.getEnvelope3D = function() {
	var obj = getEnvelope3D.apply(this, arguments);
	return new gdal.Envelope3D(obj);
};

var getExtent = gdal.Layer.prototype.getExtent;
gdal.Layer.prototype.getExtent = function() {
	var obj = getExtent.apply(this, arguments);
	return new gdal.Envelope(obj);
};

// --- add additional functionality to collections ---

function defaultForEach(iterator) {
	var n = this.count();
	for (var i = 0; i < n; i++) {
		if (iterator(this.get(i)) === false) return;
	}
}

function defaultToArray() {
	var array = [];
	this.forEach(function(geom){
		array.push(geom);
	});
	return array;
}

gdal.DatasetBands.prototype.forEach = function(iterator){
	var n = this.count();
	for (var i = 1; i <= n; i++) {
		if (iterator(this.get(i)) === false) return;
	}
};

gdal.LayerFeatures.prototype.forEach = function(iterator){
	var feature = this.first();
	while (feature) {
		if (iterator(feature) === false) return;
		feature = this.next();
	}
};

gdal.FeatureFields.prototype.forEach = function(iterator){
	var obj = this.toJSON();
	var names = Object.keys(obj);
	var n = names.length;
	for (var i = 0; i < n; i++) {
		var key   = names[i];
		var value = obj[key];
		if (iterator(value, key) === false) return;
	}
};

gdal.LayerFields.prototype.forEach = defaultForEach;
gdal.DatasetLayers.prototype.forEach = defaultForEach;
gdal.FeatureDefnFields.prototype.forEach = defaultForEach;
gdal.PolygonRings.prototype.forEach = defaultForEach;
gdal.LineStringPoints.prototype.forEach = defaultForEach;
gdal.GeometryCollectionChildren.prototype.forEach = defaultForEach;
gdal.RasterBandOverviews.prototype.forEach = defaultForEach;
gdal.GDALDrivers.prototype.forEach = defaultForEach;

gdal.GeometryCollectionChildren.prototype.toArray = defaultToArray;
gdal.LineStringPoints.prototype.toArray = defaultToArray;
gdal.PolygonRings.prototype.toArray = defaultToArray;


// add 'w' mode to gdal.open() method and also GDAL2-style driver selection
var open = gdal.open;
gdal.open = function(filename, mode, drivers, x_size, y_size, n_bands, datatype, options) {
	if (typeof drivers === 'string') {
		drivers = [drivers];
	} else if (drivers && !(drivers instanceof Array)) {
		throw new Error('driver(s) must be a string or array of strings')
	}

	if (mode === 'w') {
		// create file with given driver
		if (drivers.length !== 1) {
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
				} catch (err) { }
			});

			if (!ds) throw new Error("Error opening dataset");
			return ds;

		} else {
			// call gdal.open() method normally
			return open.apply(gdal, arguments);
		}
	}
};

function fieldTypeFromValue(val) {
	var type = typeof val;
	if (type === 'number') {
		if(val % 1 === 0) return gdal.OFTInteger;
		else return gdal.OFTReal;
	} else if (type === 'string') {
		return gdal.OFTString;
	} else if (type === 'boolean') {
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
	if (arguments.length < 2) {
		approx_ok = false;
	}
	var field_names = Object.keys(obj);
	for (var i = 0; i < field_names.length; i++) {
		var name  = field_names[i];
		var value = obj[field_names[i]];
		var type  = fieldTypeFromValue(value);
		var def   = new gdal.FieldDefn(name, type);
		this.add(def, approx_ok);
	}
};

gdal.Point.wkbType               = gdal.wkbPoint;
gdal.LineString.wkbType          = gdal.wkbLineString;
gdal.LinearRing.wkbType          = gdal.wkbLinearRing;
gdal.Polygon.wkbType             = gdal.wkbPolygon;
gdal.MultiPoint.wkbType          = gdal.wkbMultiPoint;
gdal.MultiLineString.wkbType     = gdal.wkbMultiLineString;
gdal.MultiPolygon.wkbType        = gdal.wkbMultiPolygon;
gdal.GeometryCollection.wkbType  = gdal.wkbGeometryCollection;

// enable passing geometry constructors as the geometry type
var createLayer = gdal.DatasetLayers.prototype.create;
gdal.DatasetLayers.prototype.create = function(name, srs, geom_type, creation_options) {
	if (arguments.length > 1 && srs instanceof gdal.SpatialReference) {
		arguments[1] = srs.toWKT();
	}
	if (arguments.length > 2 && geom_type instanceof Function) {
		if (typeof geom_type.wkbType === 'undefined') {
			throw new Error('Function must be a geometry constructor');
		}
		arguments[2] = geom_type.wkbType;
	}
	return createLayer.apply(this, arguments);
};