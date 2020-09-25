{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_gml_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/gml/gfstemplate.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlfeature.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlfeatureclass.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlhandler.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlpropertydefn.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlreader.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlreadstate.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlregistry.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/gmlutils.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/hugefileresolver.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/ogrgmldatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/ogrgmldriver.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/ogrgmllayer.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/parsexsd.cpp",
				"../gdal/ogr/ogrsf_frmts/gml/resolvexlinks.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/gml"
			]
		}
	]
}
