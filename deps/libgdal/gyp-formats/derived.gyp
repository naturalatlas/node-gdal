{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_derived_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/derived/deriveddataset.cpp",
				"../gdal/frmts/derived/derivedlist.c"
			],
			"include_dirs": [
				"../gdal/frmts/derived"
			]
		}
	]
}
