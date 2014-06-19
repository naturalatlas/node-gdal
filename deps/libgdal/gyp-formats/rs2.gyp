{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_rs2_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/rs2/rs2dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/rs2"
			]
		}
	]
}
