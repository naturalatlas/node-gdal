{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_nitf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/nitf/ecrgtocdataset.cpp",
				"../gdal/frmts/nitf/mgrs.c",
				"../gdal/frmts/nitf/nitf_gcprpc.cpp",
				"../gdal/frmts/nitf/nitfaridpcm.cpp",
				"../gdal/frmts/nitf/nitfbilevel.cpp",
				"../gdal/frmts/nitf/nitfdataset.cpp",
				"../gdal/frmts/nitf/nitfdes.c",
				"../gdal/frmts/nitf/nitfdump.c",
				"../gdal/frmts/nitf/nitffile.c",
				"../gdal/frmts/nitf/nitfimage.c",
				"../gdal/frmts/nitf/nitfrasterband.cpp",
				"../gdal/frmts/nitf/nitfwritejpeg.cpp",
				"../gdal/frmts/nitf/nitfwritejpeg_12.cpp",
				"../gdal/frmts/nitf/rpftocdataset.cpp",
				"../gdal/frmts/nitf/rpftocfile.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/nitf",
				"../gdal/frmts/vrt",
				"../gdal/frmts/gtiff/libtiff"
			]
		}
	]
}
