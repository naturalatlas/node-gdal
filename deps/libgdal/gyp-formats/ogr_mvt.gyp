{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_mvt_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/mvt/mvtutils.cpp",
				"../gdal/ogr/ogrsf_frmts/mvt/ogrmvtdataset.cpp",
				"../gdal/ogr/ogrsf_frmts/mvt/mvt_tile.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/mvt",
				"../gdal/ogr/ogrsf_frmts/osm"
			]
		}
	]
}
