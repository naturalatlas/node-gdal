{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_svg_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/svg/ogrsvgdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/svg/ogrsvgdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/svg/ogrsvglayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/svg"
			]
		}
	]
}
