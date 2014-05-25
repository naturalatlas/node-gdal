var path         = require('path');
var pkg          = require('../package.json');
var binding_path = path.join(__dirname, '../', pkg.binary.module_path);
var module_path  = path.join(binding_path, pkg.binary.module_name + '.node');

module.exports = require(module_path);