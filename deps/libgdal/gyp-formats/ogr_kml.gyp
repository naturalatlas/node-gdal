{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_kml_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/kml/kml.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/kmlnode.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/kmlvector.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/ogr2kmlgeometry.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/ogrkmldatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/ogrkmldriver.cpp",
				"../gdal/ogr/ogrsf_frmts/kml/ogrkmllayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/kml"
			]
		}
	]
}
