{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_til_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/til/tildataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/til",
				"../gdal/frmts/vrt"
			]
		}
	]
}
