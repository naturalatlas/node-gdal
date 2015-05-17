{
    "includes": [
        "../common.gypi"
    ],
    "targets": [
        {
            "target_name": "libgdal_mrsid_frmt",
            "type": "static_library",
            "sources": [
                "../gdal/frmts/mrsid/mrsidstream.cpp",
                "../gdal/frmts/mrsid/mrsiddataset.cpp"
            ],
            "include_dirs": [
                "../gdal/frmts/mrsid",
                "../gdal/frmts/gtiff/libgeotiff",                
                "<(mrsid_base)/Raster_DSDK/include"
            ]
        }
    ]
}