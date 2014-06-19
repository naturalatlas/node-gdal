{
	"includes": ["common.gypi"],
	"variables": {
		"shared_gdal%": "false",
		"runtime_link%": "shared"
	},
	"targets": [
		{
			"target_name": "gdal",
			"type": "loadable_module",
			"product_prefix": "",
			"product_extension": "node",
			"sources": [
				"src/node_gdal.cpp",
				"src/gdal_common.cpp",
				"src/gdal_dataset.cpp",
				"src/gdal_driver.cpp",
				"src/gdal_rasterband.cpp",
				"src/gdal_majorobject.cpp",
				"src/gdal_feature.cpp",
				"src/gdal_feature_defn.cpp",
				"src/gdal_field_defn.cpp",
				"src/gdal_geometry.cpp",
				"src/gdal_point.cpp",
				"src/gdal_linestring.cpp",
				"src/gdal_linearring.cpp",
				"src/gdal_polygon.cpp",
				"src/gdal_geometrycollection.cpp",
				"src/gdal_multipoint.cpp",
				"src/gdal_multilinestring.cpp",
				"src/gdal_multipolygon.cpp",
				"src/gdal_layer.cpp",
				"src/gdal_coordinate_transformation.cpp",
				"src/gdal_spatial_reference.cpp",
				"src/collections/dataset_bands.cpp",
				"src/collections/dataset_layers.cpp",
				"src/collections/layer_features.cpp",
				"src/collections/layer_fields.cpp",
				"src/collections/feature_fields.cpp",
				"src/collections/feature_defn_fields.cpp",
				"src/collections/geometry_collection_children.cpp",
				"src/collections/polygon_rings.cpp",
				"src/collections/linestring_points.cpp",
				"src/collections/rasterband_overviews.cpp",
				"src/collections/gdal_drivers.cpp"
			],
			"defines": [
				"PLATFORM='<(OS)'",
				"_LARGEFILE_SOURCE",
				"_FILE_OFFSET_BITS=64"
			],
			"xcode_settings": {
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
			},
			"conditions": [
				["shared_gdal == 'false'", {
					"dependencies": [
						"deps/libgdal/libgdal.gyp:libgdal"
					]
				}, {
					"conditions": [
						["runtime_link == 'static'", {
							"libraries": ["<!@(gdal-config --dep-libs)"]
						}, {
							"libraries": ["<!@(gdal-config --libs)"]
						}]
					],
					"cflags_cc": ["<!@(gdal-config --cflags)"],
					"xcode_settings": {
						"OTHER_CPLUSPLUSFLAGS":[
							"<!@(gdal-config --cflags)"
						]
					}
				}]
			]
		},
		{
			"target_name": "action_after_build",
			"type": "none",
			"dependencies": [ "<(module_name)" ],
			"copies": [
				{
					"files": [
						"<(PRODUCT_DIR)/gdal.node"
					],
					"destination": "<(module_path)"
				}
			]
		}
	]
}
