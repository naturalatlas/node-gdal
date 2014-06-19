{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ngsgeoid_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ngsgeoid/ngsgeoiddataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ngsgeoid"
			]
		}
	]
}
