{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_idrisi_frmt",
			"type": "static_library",
			"sources": [
				# "../gdal/ogr/ogrsf_frmts/idrisi/generate_test_files.c",
				"../gdal/ogr/ogrsf_frmts/idrisi/ogridrisidatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/idrisi/ogridrisidriver.cpp",
				"../gdal/ogr/ogrsf_frmts/idrisi/ogridrisilayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/idrisi",
				"../gdal/frmts/idrisi"
			]
		}
	]
}
