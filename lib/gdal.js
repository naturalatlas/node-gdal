/* eslint no-console: 0 */
var path         = require('path');
var fs           = require('fs');
var binary       = require('node-pre-gyp');
var binding_path = binary.find(path.join(__dirname, '../package.json'));
var data_path    = path.resolve(__dirname, '../deps/libgdal/gdal/data');

if (process.env.GDAL_DATA === undefined && !fs.existsSync(data_path)) {
	throw new Error("The bundled data path for node-gdal is missing '" + data_path + "' and GDAL_DATA environment is not set");
}

var gdal = module.exports = require(binding_path);

gdal.Point.Multi      = gdal.MultiPoint;
gdal.LineString.Multi = gdal.MultiLineString;
gdal.LinearRing.Multi = gdal.MultiLineString;
gdal.Polygon.Multi    = gdal.MultiPolygon;

gdal.quiet();

gdal.config = {};

/**
 * Gets a GDAL configuration setting.
 *
 * @example
 * ```
 * data_path = gdal.config.get('GDAL_DATA');```
 *
 * @for gdal
 * @static
 * @method config.get
 * @param {string} key
 * @return {string}
 */
gdal.config.get = gdal.getConfigOption;

/**
 * Sets a GDAL configuration setting.
 *
 * @example
 * ```
 * gdal.config.set('GDAL_DATA', data_path);```
 *
 * @for gdal
 * @static
 * @method config.set
 * @param {string} key
 * @param {string} value
 * @return {mixed}
 */
gdal.config.set = gdal.setConfigOption;

delete gdal.getConfigOption;
delete gdal.setConfigOption;

if (process.env.GDAL_DATA === undefined) {
	gdal.config.set('GDAL_DATA', data_path);
}

gdal.Envelope = require('./envelope.js')(gdal);
gdal.Envelope3D = require('./envelope_3d.js')(gdal);

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

function defaultForEach(callback) {
	var n = this.count();
	for (var i = 0; i < n; i++) {
		if (callback(this.get(i), i) === false) return;
	}
}

function defaultMap(callback) {
	var result = [];
	this.forEach(function(value, i) {
		result.push(callback(value, i));
	});
	return result;
}

function defaultToArray() {
	var array = [];
	this.forEach(function(geom) {
		array.push(geom);
	});
	return array;
}

/**
 * Iterates through all bands using a callback function.
 * Note: GDAL band indexes start at 1, not 0.
 *
 * @example
 * ```
 * dataset.bands.forEach(function(band, i) { ... });```
 *
 * @for gdal.DatasetBands
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.RasterBand"}}RasterBand{{/crossLink}}
 */
gdal.DatasetBands.prototype.forEach = function(callback) {
	var n = this.count();
	for (var i = 1; i <= n; i++) {
		if (callback(this.get(i), i) === false) return;
	}
};

/**
 * Iterates through all bands using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = dataset.bands.map(function(band, i) {
 *     return value;
 * });```
 *
 * @for gdal.DatasetBands
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.RasterBand"}}RasterBand{{/crossLink}}
 */
gdal.DatasetBands.prototype.map = defaultMap;

/**
 * Iterates through all features using a callback function.
 *
 * @example
 * ```
 * layer.features.forEach(function(feature, i) { ... });```
 *
 * @for gdal.LayerFeatures
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Feature"}}Feature{{/crossLink}}
 */
gdal.LayerFeatures.prototype.forEach = function(callback) {
	var i = 0;
	var feature = this.first();
	while (feature) {
		if (callback(feature, i++) === false) return;
		feature = this.next();
	}
};

/**
 * Iterates through all features using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = layer.features.map(function(band, i) {
 *     return value;
 * });```
 *
 * @for gdal.LayerFeatures
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Feature"}}Feature{{/crossLink}}
 */
gdal.LayerFeatures.prototype.map = defaultMap;

/**
 * Iterates through all fields using a callback function.
 *
 * @example
 * ```
 * layer.features.get(0).fields.forEach(function(value, key) { ... });```
 *
 * @for gdal.FeatureFields
 * @method forEach
 * @param {Function} callback The callback to be called with each feature `value` and `key`.
 */
gdal.FeatureFields.prototype.forEach = function(callback) {
	var obj = this.toObject();
	var names = Object.keys(obj);
	var n = names.length;
	for (var i = 0; i < n; i++) {
		var key = names[i];
		var value = obj[key];
		if (callback(value, key) === false) return;
	}
};

/**
 * Iterates through all fields using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = layer.features.get(0).fields.map(function(value, key) {
 *     return value;
 * });```
 *
 * @for gdal.FeatureFields
 * @method map
 * @param {Function} callback The callback to be called with each feature `value` and `key`.
 */
gdal.FeatureFields.prototype.map = defaultMap;

/**
 * Outputs the fields as a serialized JSON string.
 *
 * @for gdal.FeatureFields
 * @method toJSON
 * @return {String} Serialized JSON
 */
gdal.FeatureFields.prototype.toJSON = function() {
	return JSON.stringify(this.toObject());
};

/**
 * Converts the geometry to a GeoJSON object representation.
 *
 * @for gdal.Geometry
 * @method toObject
 * @return {Object} GeoJSON
 */
gdal.Geometry.prototype.toObject = function() {
	return JSON.parse(this.toJSON());
};

/**
 * Iterates through all field definitions using a callback function.
 *
 * @example
 * ```
 * layer.fields.forEach(function(field, i) { ... });```
 *
 * @for gdal.LayerFields
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.FieldDefn"}}FieldDefn{{/crossLink}}
 */
gdal.LayerFields.prototype.forEach = defaultForEach;

/**
 * Iterates through all field definitions using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = layer.fields.map(function(field, i) {
 *     return value;
 * });```
 *
 * @for gdal.LayerFields
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.FieldDefn"}}FieldDefn{{/crossLink}}
 */
gdal.LayerFields.prototype.map = defaultMap;

/**
 * Iterates through all layers using a callback function.
 *
 * @example
 * ```
 * dataset.layers.forEach(function(layer, i) { ... });```
 *
 * @for gdal.DatasetLayers
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Layer"}}Layer{{/crossLink}}
 */
gdal.DatasetLayers.prototype.forEach = defaultForEach;

/**
 * Iterates through all layers using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = dataset.layers.map(function(field, i) {
 *     return value;
 * });```
 *
 * @for gdal.DatasetLayers
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Layer"}}Layer{{/crossLink}}
 */
gdal.DatasetLayers.prototype.map = defaultMap;

/**
 * Iterates through all field definitions using a callback function.
 *
 * @example
 * ```
 * featureDefn.forEach(function(field, i) { ... });```
 *
 * @for gdal.FeatureDefnFields
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.FieldDefn"}}FieldDefn{{/crossLink}}
 */
gdal.FeatureDefnFields.prototype.forEach = defaultForEach;

/**
 * Iterates through all field definitions using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = dataset.layers.map(function(field, i) {
 *     return value;
 * });```
 *
 * @for gdal.FeatureDefnFields
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.FieldDefn"}}FieldDefn{{/crossLink}}
 */
gdal.FeatureDefnFields.prototype.map = defaultMap;

/**
 * Iterates through all rings using a callback function.
 *
 * @example
 * ```
 * polygon.rings.forEach(function(ring, i) { ... });```
 *
 * @for gdal.PolygonRings
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.LineString"}}LineString{{/crossLink}}
 */
gdal.PolygonRings.prototype.forEach = defaultForEach;

/**
 * Iterates through all rings using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = polygon.rings.map(function(ring, i) {
 *     return value;
 * });```
 *
 * @for gdal.LineStringPoints
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.LineString"}}LineString{{/crossLink}}
 */
gdal.PolygonRings.prototype.map = defaultMap;

/**
 * Iterates through all points using a callback function.
 *
 * @example
 * ```
 * lineString.points.forEach(function(point, i) { ... });```
 *
 * @for gdal.LineStringPoints
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Point"}}Point{{/crossLink}}
 */
gdal.LineStringPoints.prototype.forEach = defaultForEach;

/**
 * Iterates through all points using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = lineString.points.map(function(point, i) {
 *     return value;
 * });```
 *
 * @for gdal.LineStringPoints
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Point"}}Point{{/crossLink}}
 */
gdal.LineStringPoints.prototype.map = defaultMap;

/**
 * Iterates through all child geometries using a callback function.
 *
 * @example
 * ```
 * geometryCollection.children.forEach(function(geometry, i) { ... });```
 *
 * @for gdal.GeometryCollectionChildren
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Geometry"}}Geometry{{/crossLink}}
 */
gdal.GeometryCollectionChildren.prototype.forEach = defaultForEach;

/**
 * Iterates through all child geometries using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = geometryCollection.children.map(function(geometry, i) {
 *     return value;
 * });```
 *
 * @for gdal.GeometryCollectionChildren
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Geometry"}}Geometry{{/crossLink}}
 */
gdal.GeometryCollectionChildren.prototype.map = defaultMap;

/**
 * Iterates through all overviews using a callback function.
 *
 * @example
 * ```
 * band.overviews.forEach(function(overviewBand, i) { ... });```
 *
 * @for gdal.RasterBandOverviews
 * @method forEach
 * @param {Function} callback
 */
gdal.RasterBandOverviews.prototype.forEach = defaultForEach;

/**
 * Iterates through all raster overviews using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = band.overviews.map(function(overviewBand, i) {
 *     return value;
 * });```
 *
 * @for gdal.RasterBandOverviews
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Geometry"}}Geometry{{/crossLink}}
 */
gdal.RasterBandOverviews.prototype.map = defaultMap;

/**
 * Iterates through all registered drivers using a callback function.
 *
 * @example
 * ```
 * gdal.drivers.forEach(function(driver, i) { ... });```
 *
 * @for gdal.GDALDrivers
 * @method forEach
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Driver"}}Driver{{/crossLink}}
 */
gdal.GDALDrivers.prototype.forEach = defaultForEach;

/**
 * Iterates through all drivers using a callback function and builds
 * an array of the returned values.
 *
 * @example
 * ```
 * var result = gdal.drivers.map(function(driver, i) {
 *     return value;
 * });```
 *
 * @for gdal.GDALDrivers
 * @method map
 * @param {Function} callback The callback to be called with each {{#crossLink "gdal.Driver"}}Driver{{/crossLink}}
 */
gdal.GDALDrivers.prototype.map = defaultMap;

/**
 * Outputs all geometries as a regular javascript array.
 *
 * @for gdal.GeometryCollectionChildren
 * @method toArray
 * @return {Array} List of {{#crossLink "gdal.Geometry"}}Geometry{{/crossLink}} instances.
 */
gdal.GeometryCollectionChildren.prototype.toArray = defaultToArray;

/**
 * Outputs all points as a regular javascript array.
 *
 * @for gdal.LineStringPoints
 * @method toArray
 * @return {Array} List of {{#crossLink "gdal.Point"}}Point{{/crossLink}} instances.
 */
gdal.LineStringPoints.prototype.toArray = defaultToArray;

/**
 * Outputs all rings as a regular javascript array.
 *
 * @for gdal.PolygonRings
 * @method toArray
 * @return {Array} List of {{#crossLink "gdal.LineString"}}LineString{{/crossLink}} instances.
 */
gdal.PolygonRings.prototype.toArray = defaultToArray;

/**
 * Creates or opens a dataset. Dataset should be explicitly closed with `dataset.close()` method if opened in `"w"` mode to flush any changes. Otherwise, datasets are closed when (and if) node decides to garbage collect them.
 *
 * @example
 * ```
 * var dataset = gdal.open('./data.shp');```
 *
 * @for gdal
 * @throws Error
 * @method open
 * @static
 * @param {String} path Path to dataset to open
 * @param {String} [mode="r"] The mode to use to open the file: `"r"`, `"r+"`, or `"w"`
 * @param {String|Array} [drivers] Driver name, or list of driver names to attempt to use.
 *
 * @param {Integer} [x_size] Used when creating a raster dataset with the `"w"` mode.
 * @param {Integer} [y_size] Used when creating a raster dataset with the `"w"` mode.
 * @param {Integer} [band_count] Used when creating a raster dataset with the `"w"` mode.
 * @param {Integer} [data_type] Used when creating a raster dataset with the `"w"` mode.
 * @param {String[]|object} [creation_options] Used when creating a dataset with the `"w"` mode.
 *
 * @return {gdal.Dataset}
 */
gdal.open = (function() {
	var open = gdal.open;

	// add 'w' mode to gdal.open() method and also GDAL2-style driver selection
	return function(filename, mode, drivers/* , x_size, y_size, n_bands, datatype, options */) {
		if (typeof drivers === 'string') {
			drivers = [drivers];
		} else if (drivers && !Array.isArray(drivers)) {
			throw new Error('driver(s) must be a string or list of strings');
		}

		if (mode === 'w') {
			// create file with given driver
			if (!drivers) {
				throw new Error('Driver must be specified');
			}
			if (drivers.length !== 1) {
				throw new Error('Only one driver can be used to create a file');
			}
			var driver = gdal.drivers.get(drivers[0]);
			if (!driver) {
				throw new Error('Cannot find driver: ' + drivers[0]);
			}

			var args = Array.prototype.slice.call(arguments, 3); // x_size, y_size, ...
			args.unshift(filename);
			return driver.create.apply(driver, args);
		}

		if (arguments.length > 2) {
			// open file with driver list
			// loop through given drivers trying to open file
			var ds;
			drivers.forEach(function(driver_name) {
				var driver = gdal.drivers.get(driver_name);
				if (!driver) {
					throw new Error('Cannot find driver: ' + driver_name);
				}
				try {
					ds = driver.open(filename, mode);
					return false;
				} catch (err) {
					/* skip driver */
				}
			});

			if (!ds) throw new Error('Error opening dataset');
			return ds;
		}

		// call gdal.open() method normally
		return open.apply(gdal, arguments);
	};
})();

function fieldTypeFromValue(val) {
	var type = typeof val;
	if (type === 'number') {
		if (val % 1 === 0) return gdal.OFTInteger;
		return gdal.OFTReal;
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
			default : throw new Error('Array element cannot be converted into OGRFieldType');
		}
	} else if (val instanceof Buffer) {
		return gdal.OFTBinary;
	}

	throw new Error('Value cannot be converted into OGRFieldType');
}

/**
 * Creates a LayerFields instance from an object of keys and values.
 *
 * @method fromJSON
 * @for gdal.LayerFields
 * @param {Object} object
 * @param {Boolean} [approx_ok=false]
 */
gdal.LayerFields.prototype.fromJSON = (function() {
	var warned = false;
	return function(obj, approx_ok) {
		if (!warned) {
			console.warn('NODE-GDAL Deprecation Warning: LayerFields fromJSON() is deprecated, use fromObject() instead');
			warned = true;
		}
		return this.fromObject(obj, approx_ok);
	};
})();

gdal.LayerFields.prototype.fromObject = function(obj, approx_ok) {
	approx_ok = approx_ok || false;
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
gdal.DatasetLayers.prototype.create = (function() {
	var create = gdal.DatasetLayers.prototype.create;
	return function(name, srs, geom_type/* , creation_options */) {
		if (arguments.length > 2 && geom_type instanceof Function) {
			if (typeof geom_type.wkbType === 'undefined') {
				throw new Error('Function must be a geometry constructor');
			}
			arguments[2] = geom_type.wkbType;
		}
		return create.apply(this, arguments);
	};
})();

function getTypedArrayType(array) {
	if (array instanceof Uint8Array)   return 1; // gdal.GDT_Byte
	if (array instanceof Int8Array)    return 1; // gdal.GDT_Byte
	if (array instanceof Int16Array)   return 3; // gdal.GDT_Int16
	if (array instanceof Uint16Array)  return 2; // gdal.GDT_UInt16
	if (array instanceof Int32Array)   return 5; // gdal.GDT_Int32
	if (array instanceof Uint32Array)  return 4; // gdal.GDT_UInt32
	if (array instanceof Float32Array) return 6; // gdal.GDT_Float32
	if (array instanceof Float64Array) return 7; // gdal.GDT_Float64
	return 0; // gdal.GDT_Unknown
}

gdal.RasterBandPixels.prototype.read = (function() {
	var read = gdal.RasterBandPixels.prototype.read;
	return function(x, y, width, height, data, options) {
		if (!options) options = {};
		if (data) data._gdal_type = getTypedArrayType(data);
		return read.apply(this, [x, y, width, height, data, options.buffer_width, options.buffer_height, options.type, options.pixel_space, options.line_space]);
	};
})();

gdal.RasterBandPixels.prototype.write = (function() {
	var write = gdal.RasterBandPixels.prototype.write;
	return function(x, y, width, height, data, options) {
		if (!options) options = {};
		if (data) data._gdal_type = getTypedArrayType(data);
		return write.apply(this, [x, y, width, height, data, options.buffer_width, options.buffer_height, options.pixel_space, options.line_space]);
	};
})();

gdal.RasterBandPixels.prototype.readBlock = (function() {
	var readBlock = gdal.RasterBandPixels.prototype.readBlock;
	return function(x, y, data) {
		if (data) data._gdal_type = getTypedArrayType(data);
		return readBlock.apply(this, arguments);
	};
})();

gdal.RasterBandPixels.prototype.writeBlock = (function() {
	var writeBlock = gdal.RasterBandPixels.prototype.writeBlock;
	return function(x, y, data) {
		data._gdal_type = getTypedArrayType(data);
		return writeBlock.apply(this, arguments);
	};
})();
