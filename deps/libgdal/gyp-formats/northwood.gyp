{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_northwood_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/northwood/grcdataset.cpp",
				"../gdal/frmts/northwood/grddataset.cpp",
				"../gdal/frmts/northwood/northwood.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/northwood"
			]
		}
	]
}
