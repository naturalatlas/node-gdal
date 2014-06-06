{
	"includes": [
		"../../../common.gypi",
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_generic_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/generic/ogr_attrind.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogr_gensql.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogr_miattrind.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrlayerdecorator.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrlayerpool.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrmutexeddatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrmutexedlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrregisterall.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrsfdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrsfdriverregistrar.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrunionlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/generic/ogrwarpedlayer.cpp"
			],
			"include_dirs": [
				"./gdal/ogr/ogrsf_frmts/generic"
			]
		}
	]
}
