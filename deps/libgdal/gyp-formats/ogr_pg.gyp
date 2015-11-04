{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_pg_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/pg/ogrpgdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/pg/ogrpgdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/pg/ogrpglayer.cpp",
				"../gdal/ogr/ogrsf_frmts/pg/ogrpgresultlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/pg/ogrpgtablelayer.cpp",
				"../gdal/ogr/ogrsf_frmts/pg/ogrpgutility.cpp"
			],
			"include_dirs": [
				"C:/mb/windows-builds-64/packages/postgresql/src/interfaces/libpq",
				"C:/mb/windows-builds-64/packages/postgresql/src/include",
				"../gdal/ogr/ogrsf_frmts/pgdump",
				"../gdal/ogr/ogrsf_frmts/pg"
			],
			"libraries": [
				"C:/mb/windows-builds-64/packages/postgresql/src/interfaces/libpq/Release/libpq.lib"
			]
		}
	]
}
