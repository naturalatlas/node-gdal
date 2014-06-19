{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_bmp_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/bmp/bmpdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/bmp"
			]
		}
	]
}
