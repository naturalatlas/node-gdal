{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_sdts_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/sdts/ogrsdtsdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/sdts/ogrsdtsdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/sdts/ogrsdtslayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/sdts",
				"../gdal/frmts/sdts",
				"../gdal/frmts/iso8211"
			]
		}
	]
}
