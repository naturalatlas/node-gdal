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
			"./gdal/ogr/ogrsf_frmts",
			"./gdal/ogr/ogrsf_frmts/mem",
			"./gdal/ogr/ogrsf_frmts/generic",
			"./gdal/ogr/ogrsf_frmts/shape",
			"./gdal/ogr/ogrsf_frmts/avc",
			"./gdal/ogr/ogrsf_frmts/geojson/libjson"
		],
		"defines": [
			"_LARGEFILE_SOURCE",
			"_FILE_OFFSET_BITS=64",
			"PAM_ENABLED=1",
			"OGR_ENABLED=1",
			"HAVE_EXPAT=1",
			"CPU_<(endianness)_ENDIAN=1"
		],
		"cflags_cc!": ["-fno-rtti", "-fno-exceptions"],
		"cflags!": ["-fno-rtti", "-fno-exceptions"],
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