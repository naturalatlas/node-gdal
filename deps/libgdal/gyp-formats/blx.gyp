{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_blx_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/blx/blx.c",
				"../gdal/frmts/blx/blxdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/blx"
			]
		}
	]
}
