{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_htf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/htf/ogrhtfdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/htf/ogrhtfdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/htf/ogrhtflayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/htf"
			]
		}
	]
}
