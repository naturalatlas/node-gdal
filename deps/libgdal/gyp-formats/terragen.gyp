{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_terragen_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/terragen/terragendataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/terragen"
			]
		}
	]
}
