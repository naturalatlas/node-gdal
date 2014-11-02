'use strict';

var fs = require('fs');
var gdal = require('../lib/gdal.js');
var path = require('path');
var assert = require('chai').assert;
var fileUtils = require('./utils/file.js');

describe('gdal', function() {
	afterEach(gc);

	describe('reprojectImage()', function() {
		it('should write reprojected image into dst dataset', function(){


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


			var src = gdal.open(__dirname + "/data/sample.tif");
			var expected = gdal.open(__dirname + "/data/sample_warped.tif");
			var cutline_ds = gdal.open(__dirname + "/data/cutline.shp");


			//src properties
			var w =  src.rasterSize.x;
			var h =  src.rasterSize.y;
			var gt = src.geoTransform;

			//warp options
			var s_srs = src.srs; 
			var t_srs = gdal.SpatialReference.fromUserInput('EPSG:4326');
			var tr = {x: .0005, y: .0005}; // target resolution
			var tx = new gdal.CoordinateTransformation(s_srs, t_srs);
			var cutline = cutline_ds.layers.get(0).features.get(0).getGeometry();

			//transform cutline to source dataset px/line coordinates 
			var geotransformer = new gdal.CoordinateTransformation(t_srs, src);
			cutline.transform(geotransformer); 

			//compute output geotransform / dimensions
			var ul = tx.transformPoint(gt[0], gt[3]);
			var ur = tx.transformPoint(gt[0]+gt[1]*w, gt[3]);
			var lr = tx.transformPoint(gt[0]+gt[1]*w, gt[3]+gt[5]*h);
			var ll = tx.transformPoint(gt[0], gt[3]+gt[5]*h);
			
			var extent = new gdal.Polygon();
			var ring = new gdal.LinearRing();
			ring.points.add([ul,ur,lr,ll,ul]);
			extent.rings.add(ring);
			extent = extent.getEnvelope();

			var tw = Math.ceil(Math.max(extent.maxX-extent.minX)/tr.x);
			var th = Math.ceil(Math.max(extent.maxY-extent.minY)/tr.y);

			var dst = gdal.open('temp', 'w', 'MEM', tw, th, 2, gdal.GDT_Int16);
			dst.srs = t_srs;
			dst.geoTransform = [
				extent.minX, tr.x, gt[2],
				extent.maxY, gt[4], -tr.y
			];

			//warp
			gdal.reprojectImage({
				src: src,
				dst: dst,
				s_srs: s_srs,
				t_srs: t_srs,
				r: gdal.GRA_Bilinear,
				cutline: cutline,
				dstAlphaBand: 2,
				blend: 0
			});

			//compare with result of gdalwarp
		
			// gdalwarp might pick the output size slightly differently (+/- 1 px)
			assert.closeTo(dst.rasterSize.x, expected.rasterSize.x, 1);
			assert.closeTo(dst.rasterSize.y, expected.rasterSize.y, 1);
			w = Math.min(dst.rasterSize.x, expected.rasterSize.x);
			h = Math.min(dst.rasterSize.y, expected.rasterSize.y);

			//check data band
			var expected_pixels = expected.bands.get(1).pixels;
			var actual_pixels = dst.bands.get(1).pixels;
			var error = 0;
			var n = 0;
			for(var x = 0; x < w; x += 10){
				for(var y = 0; y < h; y += 10){
					error += Math.abs(actual_pixels.get(x,y)-expected_pixels.get(x,y));
					n++;
				}
			}
			var avgerror = error / n;
			assert(avgerror < 0.1, 'minimal error in pixel data');

			//check alpha band
			expected_pixels = expected.bands.get(2).pixels;
			actual_pixels = dst.bands.get(2).pixels;
			error = 0;
			for(var x = 0; x < w; x += 10){
				for(var y = 0; y < h; y += 10){
					error += Math.abs(actual_pixels.get(x,y)-expected_pixels.get(x,y));
				}
			}
			avgerror = error / n;
			assert(avgerror < 0.1, 'minimal error in alpha band pixel data');

			dst.close();
			src.close();
			cutline_ds.close();
			expected.close();
		});
	});
});