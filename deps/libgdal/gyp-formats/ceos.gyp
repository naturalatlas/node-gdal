{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ceos_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ceos/ceosdataset.cpp",
				"../gdal/frmts/ceos/ceosopen.c",
				"../gdal/frmts/ceos/ceostest.c"
			],
			"include_dirs": [
				"../gdal/frmts/ceos"
			]
		}
	]
}
