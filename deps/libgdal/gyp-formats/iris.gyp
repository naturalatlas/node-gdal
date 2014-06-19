{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_iris_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/iris/irisdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/iris"
			]
		}
	]
}
