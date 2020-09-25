{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_sigdem_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/sigdem/sigdemdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/sigdem",
				"../gdal/frmts/raw"
			]
		}
	]
}
