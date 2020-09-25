{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_geojson_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/arraylist.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/debug.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/json_c_version.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/json_object.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/json_object_iterator.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/json_tokener.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/json_util.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/linkhash.c",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson/printbuf.c",
				"../gdal/ogr/ogrsf_frmts/geojson/ogresrijsonreader.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsondatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsondriver.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonreader.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonutils.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonwritelayer.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonwriter.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrtopojsonreader.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogresrijsondriver.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonseqdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/geojson/ogrtopojsondriver.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/geojson",
				"../gdal/ogr/ogrsf_frmts/geojson/libjson"
			]
		}
	]
}
