{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_sgi_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/sgi/sgidataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/sgi"
			]
		}
	]
}
