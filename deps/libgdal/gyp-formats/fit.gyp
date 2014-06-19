{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_fit_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/fit/fit.cpp",
				"../gdal/frmts/fit/fitdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/fit"
			]
		}
	]
}
