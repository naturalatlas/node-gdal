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
				"src/gdal_dataset.cpp",
				"src/gdal_driver.cpp",
				"src/gdal_rasterband.cpp",
				"src/gdal_majorobject.cpp",
				"src/obj_cache.cpp"
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
