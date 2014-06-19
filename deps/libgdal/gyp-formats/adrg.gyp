{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_adrg_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/adrg/adrgdataset.cpp",
				"../gdal/frmts/adrg/srpdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/adrg",
				"../gdal/frmts/iso8211"
			]
		}
	]
}
