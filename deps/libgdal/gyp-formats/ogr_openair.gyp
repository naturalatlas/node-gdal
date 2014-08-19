{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_openair_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/openair/ogropenairdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/openair/ogropenairdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/openair/ogropenairlabellayer.cpp",
				"../gdal/ogr/ogrsf_frmts/openair/ogropenairlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/openair",
				"../gdal/ogr/ogrsf_frmts/xplane"
			]
		}
	]
}
