{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ingr_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ingr/IngrTypes.cpp",
				"../gdal/frmts/ingr/IntergraphBand.cpp",
				"../gdal/frmts/ingr/IntergraphDataset.cpp",
				"../gdal/frmts/ingr/JpegHelper.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ingr",
				"../gdal/frmts/gtiff/libtiff"
			]
		}
	]
}
