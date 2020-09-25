{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_raw_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/raw/ace2dataset.cpp",
				"../gdal/frmts/raw/atlsci_spheroid.cpp",
				"../gdal/frmts/raw/btdataset.cpp",
				"../gdal/frmts/raw/cpgdataset.cpp",
				"../gdal/frmts/raw/ctable2dataset.cpp",
				"../gdal/frmts/raw/dipxdataset.cpp",
				"../gdal/frmts/raw/doq1dataset.cpp",
				"../gdal/frmts/raw/doq2dataset.cpp",
				"../gdal/frmts/raw/ehdrdataset.cpp",
				"../gdal/frmts/raw/eirdataset.cpp",
				"../gdal/frmts/raw/envidataset.cpp",
				"../gdal/frmts/raw/fastdataset.cpp",
				"../gdal/frmts/raw/fujibasdataset.cpp",
				"../gdal/frmts/raw/genbindataset.cpp",
				"../gdal/frmts/raw/gscdataset.cpp",
				"../gdal/frmts/raw/gtxdataset.cpp",
				"../gdal/frmts/raw/hkvdataset.cpp",
				"../gdal/frmts/raw/idadataset.cpp",
				"../gdal/frmts/raw/krodataset.cpp",
				"../gdal/frmts/raw/landataset.cpp",
				"../gdal/frmts/raw/lcpdataset.cpp",
				"../gdal/frmts/raw/loslasdataset.cpp",
				"../gdal/frmts/raw/mffdataset.cpp",
				"../gdal/frmts/raw/ndfdataset.cpp",
				"../gdal/frmts/raw/ntv2dataset.cpp",
				"../gdal/frmts/raw/pauxdataset.cpp",
				"../gdal/frmts/raw/pnmdataset.cpp",
				"../gdal/frmts/raw/rawdataset.cpp",
				"../gdal/frmts/raw/snodasdataset.cpp",
				"../gdal/frmts/raw/roipacdataset.cpp",
				"../gdal/frmts/raw/iscedataset.cpp",
				"../gdal/frmts/raw/byndataset.cpp",
				"../gdal/frmts/raw/ntv1dataset.cpp",
				"../gdal/frmts/raw/rrasterdataset.cpp"
			],
			"include_dirs": [
				"../gdal/frmts/raw"
			]
		}
	]
}
