{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_aigrid_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/aigrid/aigccitt.c",
				"../gdal/frmts/aigrid/aigdataset.cpp",
				"../gdal/frmts/aigrid/aigopen.c",
				"../gdal/frmts/aigrid/aitest.c",
				"../gdal/frmts/aigrid/gridlib.c"
			],
			"include_dirs": [
				"../gdal/frmts/aigrid"
			]
		}
	]
}
