{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_csv_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/csv/ogrcsvdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/csv/ogrcsvdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/csv/ogrcsvlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/csv"
			]
		}
	]
}
