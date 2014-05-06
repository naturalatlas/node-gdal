{
  "variables": {
    "node_gdal_sources": [
      "src/node_gdal.cpp",
      "src/gdal_dataset.cpp",
      "src/gdal_driver.cpp",
      "src/gdal_rasterband.cpp",
      "src/gdal_majorobject.cpp"
    ]
  },
  "targets": [
    {
      "target_name": "gdal",
      "product_name": "gdal",
      "type": "loadable_module",
      "product_prefix": "",
      "product_extension":"node",
      "sources": [
        "<@(node_gdal_sources)"
      ],
      "defines": [
        "PLATFORM='<(OS)'",
        "_LARGEFILE_SOURCE",
        "_FILE_OFFSET_BITS=64"
      ],
      "libraries": [
        "<!@(gdal-config --libs)"
      ],
      "cflags": [
        "<!@(gdal-config --cflags)"
      ]
    }
  ]
}
