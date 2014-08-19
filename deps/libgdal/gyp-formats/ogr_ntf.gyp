{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_ntf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/ntf/ntf_codelist.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntf_estlayers.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntf_generic.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntf_raster.cpp",
				# "../gdal/ogr/ogrsf_frmts/ntf/ntfdump.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntffilereader.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntfrecord.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ntfstroke.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ogrntfdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ogrntfdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ogrntffeatureclasslayer.cpp",
				"../gdal/ogr/ogrsf_frmts/ntf/ogrntflayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/ntf"
			]
		}
	]
}
