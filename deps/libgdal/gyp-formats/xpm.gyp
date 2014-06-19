{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_xpm_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/xpm/xpmdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/xpm",
				"../gdal/frmts/mem"
			]
		}
	]
}
