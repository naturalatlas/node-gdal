{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_leveller_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/leveller/levellerdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/leveller"
			]
		}
	]
}
