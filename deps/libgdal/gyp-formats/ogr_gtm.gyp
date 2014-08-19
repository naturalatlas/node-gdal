{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_gtm_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/gtm/gtm.cpp",
				"../gdal/ogr/ogrsf_frmts/gtm/gtmtracklayer.cpp",
				"../gdal/ogr/ogrsf_frmts/gtm/gtmwaypointlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/gtm/ogrgtmdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/gtm/ogrgtmdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/gtm/ogrgtmlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/gtm"
			]
		}
	]
}
