{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_pgdump_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumpdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumpdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumplayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/pgdump"
			]
		}
	]
}
