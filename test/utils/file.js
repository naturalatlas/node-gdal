var fs = require('fs');

module.exports.clone = function(file) {
	var dotpos = file.lastIndexOf('.');
	var result = file.substring(0, dotpos) + '.tmp' + String(Math.random()).substring(2) + file.substring(dotpos);

	fs.writeFileSync(result, fs.readFileSync(file));
	return result;
};