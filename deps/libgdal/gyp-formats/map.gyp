{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_map_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/map/mapdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/map"
			]
		}
	]
}
