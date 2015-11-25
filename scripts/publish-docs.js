var ghpages = require('gh-pages');
var path = require('path');

process.stdout.write('Publishing to "gh-pages" branch... ');
ghpages.publish(path.resolve(__dirname, '../yuidocs'), {message: 'Updated documentation [skip ci]'}, function(err) {
	if (err) {
		process.stdout.write('error\n');
		console.error(err);
		process.exit(1);
	} else {
		process.stdout.write('success\n');
		process.exit(0);
	}
});
