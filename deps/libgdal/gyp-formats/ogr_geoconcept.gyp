{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_geoconcept_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/geoconcept/geoconcept.c",
				"../gdal/ogr/ogrsf_frmts/geoconcept/geoconcept_syscoord.c",
				"../gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/geoconcept"
			]
		}
	]
}
