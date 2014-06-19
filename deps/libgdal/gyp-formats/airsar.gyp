{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_airsar_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/airsar/airsardataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/airsar"
			]
		}
	]
}
