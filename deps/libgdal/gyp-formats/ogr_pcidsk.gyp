{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_pcidsk_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/pcidsk/ogrpcidskdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/pcidsk/ogrpcidskdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/pcidsk/ogrpcidsklayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/pcidsk",
				"../gdal/frmts/pcidsk",
				"../gdal/frmts/pcidsk/sdk"
			]
		}
	]
}
