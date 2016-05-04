{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgdal_pcidsk_frmt",
			"type": "static_library",
			"sources": [
				"../gdal/frmts/pcidsk/gdal_edb.cpp",
				"../gdal/frmts/pcidsk/pcidskdataset2.cpp",
				"../gdal/frmts/pcidsk/ogrpcidsklayer.cpp",
				"../gdal/frmts/pcidsk/sdk/channel/cbandinterleavedchannel.cpp",
				"../gdal/frmts/pcidsk/sdk/channel/cexternalchannel.cpp",
				"../gdal/frmts/pcidsk/sdk/channel/cpcidskchannel.cpp",
				"../gdal/frmts/pcidsk/sdk/channel/cpixelinterleavedchannel.cpp",
				"../gdal/frmts/pcidsk/sdk/channel/ctiledchannel.cpp",
				"../gdal/frmts/pcidsk/sdk/core/clinksegment.cpp",
				"../gdal/frmts/pcidsk/sdk/core/cpcidskfile.cpp",
				"../gdal/frmts/pcidsk/sdk/core/edb_pcidsk.cpp",
				"../gdal/frmts/pcidsk/sdk/core/libjpeg_io.cpp",
				"../gdal/frmts/pcidsk/sdk/core/metadataset_p.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidsk_pubutils.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidsk_utils.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidskbuffer.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidskcreate.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidskexception.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidskinterfaces.cpp",
				"../gdal/frmts/pcidsk/sdk/core/pcidskopen.cpp",
				"../gdal/frmts/pcidsk/sdk/core/sysvirtualfile.cpp",
				"../gdal/frmts/pcidsk/sdk/port/io_stdio.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidsk_array.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidsk_tex.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskads40model.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskapmodel.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskbinarysegment.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskbitmap.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskephemerissegment.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskgcp2segment.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskgeoref.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskpct.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskrpcmodel.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidsksegment.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidsktoutinmodel.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskvectorsegment.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/cpcidskvectorsegment_consistencycheck.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/metadatasegment_p.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/sysblockmap.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/vecsegdataindex.cpp",
				"../gdal/frmts/pcidsk/sdk/segment/vecsegheader.cpp",
				"../gdal/frmts/pcidsk/vsi_pcidsk_io.cpp"
			],
			"conditions": [
				["OS == 'win'", {
					"sources": [
						"../gdal/frmts/pcidsk/sdk/port/io_win32.cpp",
						"../gdal/frmts/pcidsk/sdk/port/win32_mutex.cpp"
					]
				}, {
					"sources": [
						"../gdal/frmts/pcidsk/sdk/port/pthread_mutex.cpp"
					]
				}]
			],
			"include_dirs": [
				"../gdal/frmts/jpeg/libjpeg",
				"../gdal/frmts/raw",
				"../gdal/frmts/pcidsk",
				"../gdal/frmts/pcidsk/sdk"
			]
		}
	]
}
