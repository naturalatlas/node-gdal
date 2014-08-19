{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_avc_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/avc/avc_bin.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_binwr.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_e00gen.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_e00parse.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_e00read.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_e00write.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_mbyte.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_misc.c",
				"../gdal/ogr/ogrsf_frmts/avc/avc_rawbin.c",
				"../gdal/ogr/ogrsf_frmts/avc/ogravcbindatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravcbindriver.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravcbinlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravcdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravce00datasource.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravce00driver.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravce00layer.cpp",
				"../gdal/ogr/ogrsf_frmts/avc/ogravclayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/avc"
			],
			"direct_dependent_settings": {
				"defines": [
					"AVCBIN_ENABLED=1"
				]
			}
		}
	]
}
