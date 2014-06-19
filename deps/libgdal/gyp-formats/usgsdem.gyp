{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_usgsdem_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/usgsdem/usgsdem_create.cpp",
				"../gdal/frmts/usgsdem/usgsdemdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/usgsdem"
			]
		}
	]
}
