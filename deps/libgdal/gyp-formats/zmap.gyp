{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_zmap_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/zmap/zmapdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/zmap"
			]
		}
	]
}
