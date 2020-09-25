{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_vrt_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/vrt/vrtdataset.cpp",
				"../gdal/frmts/vrt/vrtpansharpened.cpp",
				"../gdal/frmts/vrt/vrtderivedrasterband.cpp",
				"../gdal/frmts/vrt/vrtdriver.cpp",
				"../gdal/frmts/vrt/vrtfilters.cpp",
				"../gdal/frmts/vrt/vrtrasterband.cpp",
				"../gdal/frmts/vrt/vrtrawrasterband.cpp",
				"../gdal/frmts/vrt/vrtsourcedrasterband.cpp",
				"../gdal/frmts/vrt/vrtsources.cpp",
				"../gdal/frmts/vrt/vrtwarped.cpp",
				"../gdal/frmts/vrt/pixelfunctions.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/vrt",
				"../gdal/frmts/raw"
			]
		}
	]
}
