{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_segy_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/segy/ogrsegydatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/segy/ogrsegydriver.cpp",
				"../gdal/ogr/ogrsf_frmts/segy/ogrsegylayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/segy"
			]
		}
	]
}
