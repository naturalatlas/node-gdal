{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_null_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/null/nulldataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/null"
			]
		}
	]
}
