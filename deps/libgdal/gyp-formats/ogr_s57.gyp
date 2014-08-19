{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_s57_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/s57/ddfrecordindex.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/ogrs57datasource.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/ogrs57driver.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/ogrs57layer.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/s57classregistrar.cpp",
				# "../gdal/ogr/ogrsf_frmts/s57/s57dump.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/s57featuredefns.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/s57filecollector.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/s57reader.cpp",
				"../gdal/ogr/ogrsf_frmts/s57/s57writer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/s57",
				"../gdal/frmts/iso8211"
			]
		}
	]
}
