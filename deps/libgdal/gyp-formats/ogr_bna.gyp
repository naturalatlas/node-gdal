{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_bna_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/bna/ogrbnadatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/bna/ogrbnadriver.cpp",
				"../gdal/ogr/ogrsf_frmts/bna/ogrbnalayer.cpp",
				"../gdal/ogr/ogrsf_frmts/bna/ogrbnaparser.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/bna"
			]
		}
	]
}
