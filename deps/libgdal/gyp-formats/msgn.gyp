{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_msgn_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/msgn/msg_basic_types.cpp",
				"../gdal/frmts/msgn/msg_reader_core.cpp",
				"../gdal/frmts/msgn/msgndataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/msgn"
			]
		}
	]
}
