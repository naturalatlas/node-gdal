{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_aaigrid_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/aaigrid/aaigriddataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/aaigrid"
			]
		}
	]
}
