{
	"includes": [
		"../../common.gypi"
	],
	"variables": {
		"endianness": "<!(python -c \"import sys;print(sys.byteorder.upper())\")",
	},
	"targets": [
		{
			"target_name": "libexpat",
			"type": "static_library",
			"sources": [
				"expat/lib/xmlparse.c",
				"expat/lib/xmlrole.c",
				"expat/lib/xmltok.c",
				"expat/lib/xmltok_impl.c",
				"expat/lib/xmltok_ns.c"
			],
			"include_dirs": [
				"./expat/lib"
			],
			"conditions": [
				["OS == 'win'", {
					"include_dirs": ["./arch/win"]
				}, {
					"include_dirs": ["./arch/unix"]
				}]
			],
			"defines": [
				"XML_STATIC=1",
				"HAVE_EXPAT_CONFIG_H=1",
				"CPU_<(endianness)_ENDIAN=1"
			],
			"cflags_cc!": ["-fno-rtti", "-fno-exceptions"],
			"cflags!": ["-fno-rtti", "-fno-exceptions"],
			"xcode_settings": {
				"GCC_ENABLE_CPP_RTTI": "YES",
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
			},
			"direct_dependent_settings": {
				"include_dirs": [
					"./expat/lib",
				],
				"defines": [
					"XML_STATIC=1",
					"HAVE_EXPAT_CONFIG_H=1",
				],
				"conditions": [
					["OS == 'win'", {
						"include_dirs": ["./arch/win"]
					}, {
						"include_dirs": ["./arch/unix"]
					}]
				]
			}
		}
	]
}