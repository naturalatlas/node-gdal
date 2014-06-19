{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ers_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ers/ersdataset.cpp",
				"../gdal/frmts/ers/ershdrnode.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ers",
				"../gdal/frmts/raw"
			]
		}
	]
}
