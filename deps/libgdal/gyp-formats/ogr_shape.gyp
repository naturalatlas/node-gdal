{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_shape_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/shape/dbfopen.c",
				"../gdal/ogr/ogrsf_frmts/shape/dbfopen_wrapper.c",
				"../gdal/ogr/ogrsf_frmts/shape/ogrshapedatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/shape/ogrshapedriver.cpp",
				"../gdal/ogr/ogrsf_frmts/shape/ogrshapelayer.cpp",
				"../gdal/ogr/ogrsf_frmts/shape/sbnsearch.c",
				"../gdal/ogr/ogrsf_frmts/shape/sbnsearch_wrapper.c",
				"../gdal/ogr/ogrsf_frmts/shape/shape2ogr.cpp",
				"../gdal/ogr/ogrsf_frmts/shape/shp_vsi.c",
				"../gdal/ogr/ogrsf_frmts/shape/shpopen.c",
				"../gdal/ogr/ogrsf_frmts/shape/shpopen_wrapper.c",
				"../gdal/ogr/ogrsf_frmts/shape/shptree.c",
				"../gdal/ogr/ogrsf_frmts/shape/shptree_wrapper.c"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/shape"
			]
		}
	]
}
