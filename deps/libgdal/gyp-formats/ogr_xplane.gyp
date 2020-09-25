{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_xplane_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_apt_reader.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_awy_reader.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_fix_reader.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_nav_reader.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_reader.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogrxplanedatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogrxplanedriver.cpp",
				"../gdal/ogr/ogrsf_frmts/xplane/ogrxplanelayer.cpp"
				# "../gdal/ogr/ogrsf_frmts/xplane/test_geo_utils.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/xplane"
			]
		}
	]
}
