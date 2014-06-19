{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_jpeg_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/jpeg/jpgdataset.cpp",
				"../gdal/frmts/jpeg/libjpeg/jcapimin.c",
				"../gdal/frmts/jpeg/libjpeg/jcapistd.c",
				"../gdal/frmts/jpeg/libjpeg/jccoefct.c",
				"../gdal/frmts/jpeg/libjpeg/jccolor.c",
				"../gdal/frmts/jpeg/libjpeg/jcdctmgr.c",
				"../gdal/frmts/jpeg/libjpeg/jchuff.c",
				"../gdal/frmts/jpeg/libjpeg/jcinit.c",
				"../gdal/frmts/jpeg/libjpeg/jcmainct.c",
				"../gdal/frmts/jpeg/libjpeg/jcmarker.c",
				"../gdal/frmts/jpeg/libjpeg/jcmaster.c",
				"../gdal/frmts/jpeg/libjpeg/jcomapi.c",
				"../gdal/frmts/jpeg/libjpeg/jcparam.c",
				"../gdal/frmts/jpeg/libjpeg/jcphuff.c",
				"../gdal/frmts/jpeg/libjpeg/jcprepct.c",
				"../gdal/frmts/jpeg/libjpeg/jcsample.c",
				"../gdal/frmts/jpeg/libjpeg/jctrans.c",
				"../gdal/frmts/jpeg/libjpeg/jdapimin.c",
				"../gdal/frmts/jpeg/libjpeg/jdapistd.c",
				"../gdal/frmts/jpeg/libjpeg/jdatadst.c",
				"../gdal/frmts/jpeg/libjpeg/jdatasrc.c",
				"../gdal/frmts/jpeg/libjpeg/jdcoefct.c",
				"../gdal/frmts/jpeg/libjpeg/jdcolor.c",
				"../gdal/frmts/jpeg/libjpeg/jddctmgr.c",
				"../gdal/frmts/jpeg/libjpeg/jdhuff.c",
				"../gdal/frmts/jpeg/libjpeg/jdinput.c",
				"../gdal/frmts/jpeg/libjpeg/jdmainct.c",
				"../gdal/frmts/jpeg/libjpeg/jdmarker.c",
				"../gdal/frmts/jpeg/libjpeg/jdmaster.c",
				"../gdal/frmts/jpeg/libjpeg/jdmerge.c",
				"../gdal/frmts/jpeg/libjpeg/jdphuff.c",
				"../gdal/frmts/jpeg/libjpeg/jdpostct.c",
				"../gdal/frmts/jpeg/libjpeg/jdsample.c",
				"../gdal/frmts/jpeg/libjpeg/jdtrans.c",
				"../gdal/frmts/jpeg/libjpeg/jerror.c",
				"../gdal/frmts/jpeg/libjpeg/jfdctflt.c",
				"../gdal/frmts/jpeg/libjpeg/jfdctfst.c",
				"../gdal/frmts/jpeg/libjpeg/jfdctint.c",
				"../gdal/frmts/jpeg/libjpeg/jidctflt.c",
				"../gdal/frmts/jpeg/libjpeg/jidctfst.c",
				"../gdal/frmts/jpeg/libjpeg/jidctint.c",
				"../gdal/frmts/jpeg/libjpeg/jidctred.c",
				"../gdal/frmts/jpeg/libjpeg/jmemansi.c",
				"../gdal/frmts/jpeg/libjpeg/jmemmgr.c",
				"../gdal/frmts/jpeg/libjpeg/jquant1.c",
				"../gdal/frmts/jpeg/libjpeg/jquant2.c",
				"../gdal/frmts/jpeg/libjpeg/jutils.c",
				"../gdal/frmts/jpeg/vsidataio.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/jpeg",
				"../gdal/frmts/jpeg/libjpeg"
			]
		}
	]
}
