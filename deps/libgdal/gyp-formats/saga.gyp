{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_saga_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/saga/sagadataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/saga"
			]
		}
	]
}
