{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_elas_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/elas/elasdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/elas"
			]
		}
	]
}
