var fs = require('fs');
var path = require('path');

var copyRecursiveSync = function(src, dest) {
	var exists = fs.existsSync(src);
	var stats = exists && fs.statSync(src);
	var isDirectory = exists && stats.isDirectory();
	if (exists && isDirectory) {
		fs.mkdirSync(dest);
		fs.readdirSync(src).forEach(function(child) {
			copyRecursiveSync(path.join(src, child), path.join(dest, child));
		});
	} else {
		fs.linkSync(src, dest);
	}
};

module.exports.clone = function(file) {
	var dotpos = file.lastIndexOf('.');
	var result = file.substring(0, dotpos) + '.tmp' + String(Math.random()).substring(2) + file.substring(dotpos);

	fs.writeFileSync(result, fs.readFileSync(file));
	return result;
};

module.exports.cloneDir = function(dir) {
	var name = path.basename(dir);
	var name_new = name + '.' + String(Math.random()).substring(2) + '.tmp';
	var result = path.resolve(__dirname, '..') + '/data/temp/' + name_new;
	copyRecursiveSync(dir, result);
	return result;
};