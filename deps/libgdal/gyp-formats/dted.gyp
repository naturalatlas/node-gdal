{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_dted_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/dted/dted_api.c",
				"../gdal/frmts/dted/dted_create.c",
				"../gdal/frmts/dted/dted_ptstream.c",
				"../gdal/frmts/dted/dted_test.c",
				"../gdal/frmts/dted/dteddataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/dted"
			]
		}
	]
}
