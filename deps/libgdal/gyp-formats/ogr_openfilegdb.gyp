{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_openfilegdb_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/openfilegdb/filegdbindex.cpp",
				"../gdal/ogr/ogrsf_frmts/openfilegdb/filegdbtable.cpp",
				"../gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdbdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdbdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdblayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/openfilegdb"
			]
		}
	]
}
