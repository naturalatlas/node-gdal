{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_wasp_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/wasp/ogrwaspdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/wasp/ogrwaspdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/wasp/ogrwasplayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/wasp"
			]
		}
	]
}
