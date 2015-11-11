{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_warp_app",
			"type": "static_library",
			"sources": [				
				"../gdal/apps/commonutils.cpp",
				"../gdal/apps/gdal_utils_priv.h",
				"../gdal/apps/gdalwarp_lib.cpp"
			],
			"include_dirs": [				
			]
		}
	]
}