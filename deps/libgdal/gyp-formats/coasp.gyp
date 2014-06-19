{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_coasp_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/coasp/coasp_dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/coasp"
			]
		}
	]
}
