{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_dgn_frmt",
			"type": "static_library",
			"sources": [
				# "../gdal/ogr/ogrsf_frmts/dgn/dgndump.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnfloat.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnhelp.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnopen.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnread.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnstroke.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/dgnwrite.cpp",
				# "../gdal/ogr/ogrsf_frmts/dgn/dgnwritetest.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/ogrdgndatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/ogrdgndriver.cpp",
				"../gdal/ogr/ogrsf_frmts/dgn/ogrdgnlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/dgn"
			]
		}
	]
}
