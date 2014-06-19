{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_xyz_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/xyz/xyzdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/xyz"
			]
		}
	]
}
