{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_rmf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/rmf/rmfdataset.cpp",
				"../gdal/frmts/rmf/rmfjpeg.cpp",
				"../gdal/frmts/rmf/rmfdem.cpp",
				"../gdal/frmts/rmf/rmflzw.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/rmf"
			]
		}
	]
}
