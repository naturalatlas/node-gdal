{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_segukooa_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooadatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooadriver.cpp",
				"../gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooalayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/segukooa"
			]
		}
	]
}
