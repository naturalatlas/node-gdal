{
	"variables": {
		"endianness": "<!(python -c \"import sys;print(sys.byteorder.upper())\")",
	},
	"target_defaults": {
		"include_dirs": [
			"./gdal/alg",
			"./gdal/gcore",
			"./gdal/port",
			"./gdal/bridge",
			"./gdal/frmts",	
			"./gdal/frmts/gtiff",
			"./gdal/ogr",
			"./gdal/ogr/ogrsf_frmts/shape",
			"./gdal/ogr/ogrsf_frmts/avc",
			"./gdal/ogr/ogrsf_frmts/geojson/libjson"
		],
		"defines": [
			"_LARGEFILE_SOURCE",
			"_FILE_OFFSET_BITS=64",
			"CPU_<(endianness)_ENDIAN=1"
		],
		"cflags_cc!": ["-fno-rtti"],
		"conditions": [
			["OS == 'win'", {
				"include_dirs": ["./arch/win"]
			}],
			["OS == 'freebsd'", {
				"include_dirs": ["./arch/bsd"]
			}],
			["OS != 'freebsd' and OS != 'win'", {
				"include_dirs": ["./arch/unix"]
			}],
			["OS == 'mac'", {
				"xcode_settings": {
					"GCC_ENABLE_CPP_RTTI": "YES",
					"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
				}
			}]
		],
	}
}