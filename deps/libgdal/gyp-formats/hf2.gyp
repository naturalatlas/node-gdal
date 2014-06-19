{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_hf2_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/hf2/hf2dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/hf2"
			]
		}
	]
}
