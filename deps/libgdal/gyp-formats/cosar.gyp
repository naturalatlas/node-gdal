{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_cosar_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/cosar/cosar_dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/cosar"
			]
		}
	]
}
