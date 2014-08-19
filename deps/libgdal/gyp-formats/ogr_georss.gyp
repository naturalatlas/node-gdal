{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_georss_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/georss/ogrgeorssdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/georss/ogrgeorssdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/georss/ogrgeorsslayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/georss"
			]
		}
	]
}
