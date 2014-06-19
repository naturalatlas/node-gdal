{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ceos2_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/ceos2/ceos.c",
				"../gdal/frmts/ceos2/ceosrecipe.c",
				"../gdal/frmts/ceos2/ceossar.c",
				"../gdal/frmts/ceos2/link.c",
				"../gdal/frmts/ceos2/sar_ceosdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/ceos2",
				"../gdal/frmts/raw"
			]
		}
	]
}
