{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_jdem_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/jdem/jdemdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/jdem"
			]
		}
	]
}
