var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal', function() {
	afterEach(gc);

	describe('contourGenerate()', function() {
		var src, srcband, dst, lyr;

		before(function() {
			// create a simple ramp in memory
			var w = 64;
			var h = 64;
			src = gdal.open('temp', 'w', 'MEM', w, h, 1);
			srcband = src.bands.get(1);
			for (var y = 0; y < h; y++) {
				var buf = Buffer.alloc(w);
				buf.fill(y * 4);
				srcband.pixels.write(0, y, w, 1, new Uint8Array(buf));
			}
		});
		after(function() {
			src.close();
		});
		beforeEach(function() {
			dst = gdal.open('temp', 'w', 'Memory');

			lyr = dst.layers.create('temp', null, gdal.Linestring);
			lyr.fields.add(new gdal.FieldDefn('id', gdal.OFTInteger));
			lyr.fields.add(new gdal.FieldDefn('elev', gdal.OFTReal));
		});
		afterEach(function() {
			try {
				dst.close();
			} catch (err) {
				/* ignore */
			}
		});
		it('should generate contours when passed an interval / base', function() {
			var offset = 7;
			var interval = 32;

			gdal.contourGenerate({
				src: srcband,
				dst: lyr,
				offset: offset,
				interval: interval,
				idField: 0,
				elevField: 1
			});

			assert(lyr.features.count() > 0, 'features were created');

			lyr.features.forEach(function(feature) {
				assert((feature.fields.get('elev') - offset) % interval === 0, 'contour used correct interval / base');
				assert.isFalse(feature.getGeometry().isEmpty());
			});
		});
		it('should accept an array of fixed levels', function() {
			var levels = [53, 43, 193].sort();

			gdal.contourGenerate({
				src: srcband,
				dst: lyr,
				fixedLevels: levels,
				idField: 0,
				elevField: 1
			});

			assert(lyr.features.count() > 0, 'features were created');

			var actual_levels = [];

			lyr.features.forEach(function(feature) {
				var elev = feature.fields.get('elev');
				assert.include(levels, elev, 'contour elevation in array of fixed levels');
				assert.isFalse(feature.getGeometry().isEmpty());
				if (actual_levels.indexOf(elev) === -1) actual_levels.push(elev);
			});

			assert.deepEqual(levels, actual_levels.sort(), 'all fixed levels used');
		});
	});
	describe('fillNodata()', function() {
		var src, srcband;
		var holes_x = [53, 61, 61, 1, 43, 44, 5];
		var holes_y = [11, 5, 6, 33, 22, 11, 0];
		var nodata = 33;

		beforeEach(function() {
			var w = 64;
			var h = 64;

			src = gdal.open('temp', 'w', 'MEM', w, h, 1);
			srcband = src.bands.get(1);
			srcband.noDataValue = nodata;

			// sprinkle a solid fill with nodata
			var buf = Buffer.alloc(w * h);
			buf.fill(128);
			srcband.pixels.write(0, 0, w, h, new Uint8Array(buf));
			for (var i = 0; i < holes_x.length; i++) {
				srcband.pixels.set(holes_x[i], holes_y[i], nodata);
				assert.equal(srcband.pixels.get(holes_x[i], holes_y[i]), nodata);
			}
		});
		afterEach(function() {
			try {
				src.close();
			} catch (err) {
				/* ignore */
			}
		});
		it('should fill nodata values', function() {
			gdal.fillNodata({
				src: srcband,
				searchDist: 3,
				smoothingIterations: 2
			});

			for (var i = 0; i < holes_x.length; i++) {
				assert.notEqual(srcband.pixels.get(holes_x[i], holes_y[i]), nodata);
			}
		});
	});
	describe('checksumImage()', function() {
		var src, band;
		var w = 16;
		var h = 16;
		beforeEach(function() {
			src = gdal.open('temp', 'w', 'MEM', w, h, 1);
			band = src.bands.get(1);
		});
		afterEach(function() {
			try {
				src.close();
			} catch (err) {
				/* ignore */
			}
		});
		it('should generate unique checksum for the given region', function() {
			for (var x = 0; x < w; x++) {
				for (var y = 0; y < h; y++) {
					band.pixels.set(x, y, (x * h + y) % 255);
				}
			}
			band.pixels.set(4, 4, 25);
			var a = gdal.checksumImage(band);
			band.pixels.set(4, 4, 93);
			var b = gdal.checksumImage(band);
			var c = gdal.checksumImage(band, 8, 0, w / 2, h);

			assert.notEqual(a, b);
			assert.notEqual(b, c);
		});
	});
	describe('sieveFilter()', function() {
		var src, band;
		var w = 64;
		var h = 64;
		beforeEach(function() {
			src = gdal.open('temp', 'w', 'MEM', w, h, 1);
			band = src.bands.get(1);

			// create two rectangles next to eachother of differing sizes
			var small_buffer = Buffer.alloc(4 * 4);
			small_buffer.fill(10);
			var big_buffer = Buffer.alloc(32 * 32);
			big_buffer.fill(20);

			band.pixels.write(5, 5, 32, 32, new Uint8Array(big_buffer));
			band.pixels.write(7, 7, 4, 4, new Uint8Array(small_buffer));
		});
		afterEach(function() {
			try {
				src.close();
			} catch (err) {
				/* ignore */
			}
		});
		it('should fill smaller polygons with value from neighboring bigger polygon', function() {
			assert.equal(band.pixels.get(8, 8), 10);

			gdal.sieveFilter({
				src: band,
				dst: band, // in place
				threshold: 4 * 4 + 1,
				connectedness: 8
			});

			assert.equal(band.pixels.get(8, 8), 20);
		});
	});
	describe('polygonize()', function() {
		var src, srcband, dst, lyr;

		before(function() {
			// create two identical rectangles
			var w = 64;
			var h = 64;
			src = gdal.open('temp', 'w', 'MEM', w, h, 1);
			srcband = src.bands.get(1);
			for (var y = 0; y < h; y++) {
				var buf = Buffer.alloc(w);
				buf.fill(y & 32);
				srcband.pixels.write(0, y, w, 1, new Uint8Array(buf));
			}
		});
		after(function() {
			try {
				src.close();
			} catch (err) {
				/* ignore */
			}
		});
		beforeEach(function() {
			dst = gdal.open('temp', 'w', 'Memory');
			lyr = dst.layers.create('temp', null, gdal.Polygon);
			lyr.fields.add(new gdal.FieldDefn('val', gdal.OFTInteger));
		});
		afterEach(function() {
			try {
				dst.close();
			} catch (err) {
				/* ignore */
			}
		});
		it('should generate polygons from a RasterBand', function() {
			gdal.polygonize({
				src: srcband,
				dst: lyr,
				pixValField: 0,
				connectedness: 8
			});

			assert.equal(lyr.features.count(), 2);
			lyr.features.forEach(function(f) {
				var geom = f.getGeometry();
				assert.isFalse(geom.isEmpty());
				assert.instanceOf(geom, gdal.Polygon);
			});
		});
	});
});
