var path         = require('path');
var pkg          = require('../package.json');
var binding_path = path.join(__dirname, '../', pkg.binary.module_path);
var module_path  = path.join(binding_path, pkg.binary.module_name + '.node');

var gdal = module.exports = require(module_path);
var ogr  = gdal.ogr;

// --- migrate following to c++ eventually ---

ogr.Feature.prototype.getFields = function() {
	var i, n, values, field_name;

	n = this.getFieldCount();
	values = {};
	for (i = 0; i < n; i++) {
		field_name = this.getFieldDefn(i).getName();
		values[field_name] = this.getField(i);
	}
	return values;
};

ogr.Feature.prototype.setFields = function(values) {
	var i, n, values, field_name;

	n = this.getFieldCount();
	for (i = 0; i < n; i++) {
		field_name = this.getFieldDefn(i).getName();
		this.setField(field_name, values[field_name]);
	}
};