{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_e00grid_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/e00grid/e00griddataset.cpp",
				"../gdal/frmts/e00grid/e00read.c"
			],
			"include_dirs": [
				"../gdal/frmts/e00grid"
			]
		}
	]
}
