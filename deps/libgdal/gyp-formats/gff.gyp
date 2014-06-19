{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_gff_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/gff/gff_dataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/gff"
			]
		}
	]
}
