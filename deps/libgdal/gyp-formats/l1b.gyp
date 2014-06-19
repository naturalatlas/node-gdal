{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_l1b_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/l1b/l1bdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/l1b"
			]
		}
	]
}
