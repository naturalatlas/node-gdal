{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_rec_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/rec/ll_recio.cpp",
				"../gdal/ogr/ogrsf_frmts/rec/ogrrecdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/rec/ogrrecdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/rec/ogrreclayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/rec"
			]
		}
	]
}
