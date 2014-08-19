{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_vrt_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/vrt/ogrvrtdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/vrt/ogrvrtdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/vrt/ogrvrtlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/vrt"
			]
		}
	]
}
