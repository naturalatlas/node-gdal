{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_kmlsuperoverlay_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/kmlsuperoverlay/kmlsuperoverlaydataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/kmlsuperoverlay"
			]
		}
	]
}
