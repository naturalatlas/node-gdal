{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_pds_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/pds/isis2dataset.cpp",
				"../gdal/frmts/pds/isis3dataset.cpp",
				"../gdal/frmts/pds/nasakeywordhandler.cpp",
				"../gdal/frmts/pds/vicarkeywordhandler.cpp",
				"../gdal/frmts/pds/vicardataset.cpp",
				"../gdal/frmts/pds/pdsdataset.cpp",
				"../gdal/frmts/pds/pds4dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/pds",
				"../gdal/frmts/raw"
			]
		}
	]
}
