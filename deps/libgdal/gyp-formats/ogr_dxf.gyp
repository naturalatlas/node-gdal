{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_ogr_dxf_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/ogr/ogrsf_frmts/dxf/intronurbs.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogr_autocad_services.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_blockmap.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_dimension.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_hatch.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_feature.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_leader.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_ocstransformer.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxf_polyline_smooth.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfblockslayer.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfblockswriterlayer.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfdatasource.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfdriver.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxflayer.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfreader.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfwriterds.cpp",
				"../gdal/ogr/ogrsf_frmts/dxf/ogrdxfwriterlayer.cpp"
			],
			"include_dirs": [
				"../gdal/ogr/ogrsf_frmts/dxf"
			]
		}
	]
}
