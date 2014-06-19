{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_jaxapalsar_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/jaxapalsar/jaxapalsardataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/jaxapalsar"
			]
		}
	]
}
