{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_hfa_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/hfa/hfa_overviews.cpp",
				"../gdal/frmts/hfa/hfaband.cpp",
				"../gdal/frmts/hfa/hfacompress.cpp",
				"../gdal/frmts/hfa/hfadataset.cpp",
				"../gdal/frmts/hfa/hfadictionary.cpp",
				"../gdal/frmts/hfa/hfaentry.cpp",
				"../gdal/frmts/hfa/hfafield.cpp",
				"../gdal/frmts/hfa/hfaopen.cpp",
				"../gdal/frmts/hfa/hfatype.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/hfa"
			]
		}
	]
}
