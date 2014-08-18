{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_sxf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/sxf/ogrsxfdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/sxf/ogrsxfdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/sxf/ogrsxflayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/sxf"
			]
		}
	]
}
