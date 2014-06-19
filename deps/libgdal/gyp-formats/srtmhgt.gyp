{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_srtmhgt_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/srtmhgt/srtmhgtdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/srtmhgt"
			]
		}
	]
}
