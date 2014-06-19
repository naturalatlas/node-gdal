{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ilwis_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ilwis/ilwiscoordinatesystem.cpp",
				"../gdal/frmts/ilwis/ilwisdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ilwis"
			]
		}
	]
}
