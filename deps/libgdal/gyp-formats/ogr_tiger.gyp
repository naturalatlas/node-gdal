{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_tiger_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/tiger/ogrtigerdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/ogrtigerdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/ogrtigerlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigeraltname.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerarealandmarks.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigercompletechain.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerentitynames.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerfeatureids.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerfilebase.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigeridhistory.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerinfo.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerkeyfeatures.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerlandmarks.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigeroverunder.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpip.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpoint.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpolychainlink.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpolygon.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpolygoncorrections.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerpolygoneconomic.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerspatialmetadata.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigertlidrange.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerzerocellid.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerzipcodes.cpp",
				"../gdal/ogr/ogrsf_frmts/tiger/tigerzipplus4.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/tiger"
			]
		}
	]
}
