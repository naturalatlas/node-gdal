var gdal = require('../lib/gdal.js');
var assert = require('chai').assert;

describe('gdal', function() {
	afterEach(gc);

	describe('suggestedWarpOutput()', function() {
		var src;
		beforeEach(function() {
			src = gdal.open(__dirname + '/data/sample.tif');
		});
		afterEach(function() {
			src.close();
		});
		it('should return object with suggested output dimensions', function() {
			// src properties
			var w =  src.rasterSize.x;
			var h =  src.rasterSize.y;
			var gt = src.geoTransform;

			// warp options
			var s_srs = src.srs;
			var t_srs = gdal.SpatialReference.fromUserInput('EPSG:4326');
			var tx = new gdal.CoordinateTransformation(s_srs, t_srs);

			// compute output extent
			var ul = tx.transformPoint(gt[0], gt[3]);
			var ur = tx.transformPoint(gt[0] + gt[1] * w, gt[3]);
			var lr = tx.transformPoint(gt[0] + gt[1] * w, gt[3] + gt[5] * h);
			var ll = tx.transformPoint(gt[0], gt[3] + gt[5] * h);

			var extent = new gdal.Polygon();
			var ring = new gdal.LinearRing();
			ring.points.add([ul, ur, lr, ll, ul]);
			extent.rings.add(ring);
			extent = extent.getEnvelope();

			// compute pixel resolution in target srs (function assumes square pixels)

			var s_diagonal = new gdal.LineString();
			s_diagonal.points.add(gt[0], gt[3]);
			s_diagonal.points.add(gt[0] + gt[1] * w, gt[3] + gt[5] * h);
			var t_diagonal = s_diagonal.clone();
			t_diagonal.transform(tx);

			var pixels_along_diagonal = Math.sqrt(w * w + h * h);
			// var sr = s_diagonal.getLength() / pixels_along_diagonal;
			var tr = t_diagonal.getLength() / pixels_along_diagonal;

			// compute expected size / geotransform with computed resolution

			var expected = {
				geoTransform: [
					extent.minX, tr, gt[2],
					extent.maxY, gt[4], -tr
				],
				rasterSize: {
					x: Math.ceil(Math.max(extent.maxX - extent.minX) / tr),
					y: Math.ceil(Math.max(extent.maxY - extent.minY) / tr)
				}
			};

			var output = gdal.suggestedWarpOutput({
				src: src,
				s_srs: s_srs,
				t_srs: t_srs
			});

			assert.closeTo(output.rasterSize.x, expected.rasterSize.x, 1);
			assert.closeTo(output.rasterSize.y, expected.rasterSize.y, 1);
			assert.closeTo(output.geoTransform[0], expected.geoTransform[0], 0.001);
			assert.closeTo(output.geoTransform[1], expected.geoTransform[1], 0.001);
			assert.closeTo(output.geoTransform[2], expected.geoTransform[2], 0.001);
			assert.closeTo(output.geoTransform[3], expected.geoTransform[3], 0.001);
			assert.closeTo(output.geoTransform[4], expected.geoTransform[4], 0.001);
			assert.closeTo(output.geoTransform[5], expected.geoTransform[5], 0.001);
		});
	});
	describe('reprojectImage()', function() {
		var src;
		beforeEach(function() {
			src = gdal.open(__dirname + '/data/sample.tif');
		});
		afterEach(function() {
			try { src.close(); } catch (err) { /* ignore */ }
		});
		it('should write reprojected image into dst dataset', function() {
			var x, y;

			/*
			 * expected result is the same (but not necessarily exact) as the result of:
			 *
			 *  gdalwarp
			 * -tr 0.0005 0.0005
			 * -t_srs EPSG:4326
			 * -r bilinear
			 * -cutline ./test/data/cutline.shp
			 * -dstalpha
			 * -of GTiff
			 * ./test/data/sample.tif ./test/data/sample_warped.tif
			 */

			var expected = gdal.open(__dirname + '/data/sample_warped.tif');
			var cutline_ds = gdal.open(__dirname + '/data/cutline.shp');

			// src properties
			var w =  src.rasterSize.x;
			var h =  src.rasterSize.y;
			var gt = src.geoTransform;

			// warp options
			var s_srs = src.srs;
			var t_srs = gdal.SpatialReference.fromUserInput('EPSG:4326');
			var tr = {x: 0.0005, y: 0.0005}; // target resolution
			var tx = new gdal.CoordinateTransformation(s_srs, t_srs);
			var cutline = cutline_ds.layers.get(0).features.get(0).getGeometry();

			// transform cutline to source dataset px/line coordinates
			var geotransformer = new gdal.CoordinateTransformation(t_srs, src);
			cutline.transform(geotransformer);

			// compute output geotransform / dimensions
			var ul = tx.transformPoint(gt[0], gt[3]);
			var ur = tx.transformPoint(gt[0] + gt[1] * w, gt[3]);
			var lr = tx.transformPoint(gt[0] + gt[1] * w, gt[3] + gt[5] * h);
			var ll = tx.transformPoint(gt[0], gt[3] + gt[5] * h);

			var extent = new gdal.Polygon();
			var ring = new gdal.LinearRing();
			ring.points.add([ul, ur, lr, ll, ul]);
			extent.rings.add(ring);
			extent = extent.getEnvelope();

			var tw = Math.ceil(Math.max(extent.maxX - extent.minX) / tr.x);
			var th = Math.ceil(Math.max(extent.maxY - extent.minY) / tr.y);

			var dst = gdal.open('temp', 'w', 'MEM', tw, th, 2, gdal.GDT_Int16);
			dst.srs = t_srs;
			dst.geoTransform = [
				extent.minX, tr.x, gt[2],
				extent.maxY, gt[4], -tr.y
			];

			// warp
			gdal.reprojectImage({
				src: src,
				dst: dst,
				s_srs: s_srs,
				t_srs: t_srs,
				resampling: gdal.GRA_Bilinear,
				cutline: cutline,
				dstAlphaBand: 1,
				blend: 0,
				srcBands: [1],
				dstBands: [2]
			});

			// compare with result of gdalwarp

			// gdalwarp might pick the output size slightly differently (+/- 1 px)
			assert.closeTo(dst.rasterSize.x, expected.rasterSize.x, 1);
			assert.closeTo(dst.rasterSize.y, expected.rasterSize.y, 1);
			w = Math.min(dst.rasterSize.x, expected.rasterSize.x);
			h = Math.min(dst.rasterSize.y, expected.rasterSize.y);

			// check data band
			var expected_pixels = expected.bands.get(1).pixels;
			var actual_pixels = dst.bands.get(2).pixels;
			var error = 0;
			var n = 0;
			for (x = 0; x < w; x += 10) {
				for (y = 0; y < h; y += 10) {
					error += Math.abs(actual_pixels.get(x, y) - expected_pixels.get(x, y));
					n++;
				}
			}
			var avgerror = error / n;
			assert.isBelow(avgerror, 0.5, 'minimal error in pixel data');

			// check alpha band - skipped for now (breaks on shared MacOS builds on travis)
			/*
			expected_pixels = expected.bands.get(2).pixels;
			actual_pixels = dst.bands.get(1).pixels;
			error = 0;
			for (x = 0; x < w; x += 10) {
				for (y = 0; y < h; y += 10) {
					error += Math.abs(actual_pixels.get(x, y) - expected_pixels.get(x, y));
				}
			}
			avgerror = error / n;
			assert.isBelow(avgerror, 0.5, 'minimal error in alpha band pixel data');
			*/

			dst.close();
			cutline_ds.close();
			expected.close();
		});
		it('should use approx transformer if maxError is given', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};
			var info = gdal.suggestedWarpOutput(options);

			// use lower than suggested resolution (faster)
			info.rasterSize.x /= 4;
			info.rasterSize.y /= 4;
			info.geoTransform[1] *= 4;
			info.geoTransform[5] *= 4;

			// compute exact version
			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			gdal.reprojectImage(options);

			var exact_checksum = gdal.checksumImage(options.dst.bands.get(1));

			// compute approximate version
			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;
			options.maxError = 4;

			gdal.reprojectImage(options);

			var approx_checksum = gdal.checksumImage(options.dst.bands.get(1));


			assert.notEqual(approx_checksum, exact_checksum);
		});
		it('should produce same result using multi option', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};
			var info = gdal.suggestedWarpOutput(options);

			// use lower than suggested resolution (faster)
			info.rasterSize.x /= 4;
			info.rasterSize.y /= 4;
			info.geoTransform[1] *= 4;
			info.geoTransform[5] *= 4;

			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			gdal.reprojectImage(options);

			var expected_checksum = gdal.checksumImage(options.dst.bands.get(1));

			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;
			options.multi = true;

			gdal.reprojectImage(options);

			var result_checksum = gdal.checksumImage(options.dst.bands.get(1));

			assert.equal(result_checksum, expected_checksum);
		});
		it('should throw if cutline is wrong geometry type', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				cutline: new gdal.LineString()
			};
			var info = gdal.suggestedWarpOutput(options);

			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			});
		});
		it('should throw if src dataset has been closed', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};
			var info = gdal.suggestedWarpOutput(options);

			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			src.close();

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'src dataset already closed');
		});
		it('should throw if dst dataset has been closed', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};
			var info = gdal.suggestedWarpOutput(options);

			options.dst = gdal.open('temp', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			options.dst.close();

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'dst dataset already closed');
		});
		it('should throw if dst dataset isnt a raster', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};

			options.dst = gdal.open('temp', 'w', 'Memory');

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, /must be a raster dataset|There is no affine transformation and no GCPs/);
		});
		it('should throw if src dataset isnt a raster', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326)
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			options.src = gdal.open('temp_src', 'w', 'Memory');

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, /must be a raster dataset|There is no affine transformation and no GCPs/);
		});
		it('should throw if srcBands option is provided but dstBands isnt', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				srcBands: [1]
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'dstBands must be provided if srcBands option is used');
		});
		it('should throw if dstBands option is provided but srcBands isnt', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				dstBands: [1]
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'srcBands must be provided if dstBands option is used');
		});
		it('should throw if srcBands option is invalid', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				srcBands: [3],
				dstBands: [1]
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'out of range for dataset');
		});
		it('should throw if dstBands option is invalid', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				srcBands: [1],
				dstBands: [3]
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'out of range for dataset');
		});
		it('should throw if dstAlphaBand is invalid', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				dstAlphaBand: 5
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'out of range for dataset');
		});
		it('should throw if memoryLimit is invalid', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				memoryLimit: 1
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			assert.throws(function() {
				gdal.reprojectImage(options);
			}, 'dfWarpMemoryLimit=1 is unreasonably small');
		});

		it('should use additional stringlist options', function() {
			var options = {
				src: src,
				s_srs: src.srs,
				t_srs: gdal.SpatialReference.fromEPSG(4326),
				options: {INIT_DEST: 123}
			};

			var info = gdal.suggestedWarpOutput(options);
			options.dst = gdal.open('temp_dst', 'w', 'MEM', info.rasterSize.x, info.rasterSize.y, 1, gdal.GDT_Byte);
			options.dst.geoTransform = info.geoTransform;

			gdal.reprojectImage(options);

			var value = options.dst.bands.get(1).pixels.get(0, 0);

			assert.equal(value, 123);
		});

		function greaterThan2(version) {
			var parts = version.split('.').map(function(part) { return +part; } );
			if (parts[0] >= 2) {
				return true;
			}
			return false;
		}

		if (greaterThan2(gdal.version)) {
			it('should throw error if GDAL can\'t create transformer', function() {
				src = gdal.open(__dirname + '/data/unsupported-srs.tif');

				var options = {
					src: src,
					s_srs: src.srs,
					t_srs: gdal.SpatialReference.fromEPSG(3857)
				};

				assert.throws(function() {
					gdal.suggestedWarpOutput(options);
				}, 'Mercator_1SP with scale != 1.0 and latitude of origin != 0, not supported by PROJ.4.');
			});
		} else {
			it.skip('should throw error if GDAL can\'t create transformer', function() {});
		}
	});
});
