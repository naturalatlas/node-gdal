{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_dimap_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/dimap/dimapdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/dimap"
			]
		}
	]
}
