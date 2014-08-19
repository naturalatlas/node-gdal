{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_mem_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/mem/ogrmemdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/mem/ogrmemdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/mem/ogrmemlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/mem"
			]
		}
	]
}
