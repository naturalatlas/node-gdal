{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_edigeo_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/edigeo/ogredigeodatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/edigeo/ogredigeodriver.cpp",
				"../gdal/ogr/ogrsf_frmts/edigeo/ogredigeolayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/edigeo"
			]
		}
	]
}
