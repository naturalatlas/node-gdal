{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_mitab_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_bounds.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_coordsys.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_datfile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_feature.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_feature_mif.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_geometry.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_idfile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_imapinfofile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_indfile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_mapcoordblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_mapfile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_mapheaderblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_mapindexblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_mapobjectblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_maptoolblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_middatafile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_miffile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_ogr_datasource.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_ogr_driver.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_rawbinblock.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_spatialref.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_tabfile.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_tabseamless.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_tabview.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_tooldef.cpp",
				"../gdal/ogr/ogrsf_frmts/mitab/mitab_utils.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/mitab"
			]
		}
	]
}
