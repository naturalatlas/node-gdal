{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_aeronavfaa_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaadatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaadriver.cpp",
				"../gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaalayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/aeronavfaa"
			]
		}
	]
}
