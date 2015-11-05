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
				"<!@(pg_config --includedir)",
				"C:/Program Files/PostgreSQL/9.4/include",
				"../gdal/ogr/ogrsf_frmts/pgdump",
				"../gdal/ogr/ogrsf_frmts/pg"
			],
			"libraries": [
				"-lpq -L<!@(pg_config --libdir)",
				"libpq.lib"
			],
			"VCLinkerTool": {
				"AdditionalLibraryDirectories": [
					"C:/Program Files/PostgreSQL/9.4/lib"
				]
			}
		}
	]
}
