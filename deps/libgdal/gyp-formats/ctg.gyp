{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ctg_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ctg/ctgdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ctg"
			]
		}
	]
}
