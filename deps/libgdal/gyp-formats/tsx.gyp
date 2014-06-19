{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_tsx_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/tsx/tsxdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/tsx"
			]
		}
	]
}
