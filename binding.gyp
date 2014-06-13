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
				"src/node_ogr.cpp",
				"src/ogr_datasource.cpp",
				"src/ogr_driver.cpp",
				"src/ogr_feature.cpp",
				"src/ogr_feature_defn.cpp",
				"src/ogr_field_defn.cpp",
				"src/ogr_geometry.cpp",
				"src/ogr_point.cpp",
				"src/ogr_linestring.cpp",
				"src/ogr_linearring.cpp",
				"src/ogr_polygon.cpp",
				"src/ogr_geometrycollection.cpp",
				"src/ogr_multipoint.cpp",
				"src/ogr_multilinestring.cpp",
				"src/ogr_multipolygon.cpp",
				"src/ogr_layer.cpp",
				"src/ogr_coordinate_transformation.cpp",
				"src/ogr_spatial_reference.cpp",
				"src/collections/rasterband.cpp",
				"src/collections/layer.cpp",
				"src/collections/feature.cpp",
				"src/collections/field.cpp",
				"src/collections/layer_field_defn.cpp",
				"src/collections/field_defn.cpp"
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
