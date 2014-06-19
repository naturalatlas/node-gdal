{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_idrisi_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/idrisi/IdrisiDataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/idrisi"
			]
		}
	]
}
