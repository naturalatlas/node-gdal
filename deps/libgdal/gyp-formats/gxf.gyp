{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_gxf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/gxf/gxf_ogcwkt.c",
				"../gdal/frmts/gxf/gxf_proj4.c",
				"../gdal/frmts/gxf/gxfdataset.cpp",
				"../gdal/frmts/gxf/gxfopen.c"
			],
			"include_dirs": [
				"../gdal/frmts/gxf"
			]
		}
	]
}
