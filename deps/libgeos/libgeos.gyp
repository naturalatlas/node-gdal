{
	"includes": [
		"../../common.gypi"
	],
	"targets": [
		{
			"target_name": "libgeos",
			"type": "static_library",
			"sources": [
				# python 2 doesnt have recursive globs! deepest is 4 levels, so do it hacky
				'<!@(python ../glob-files.py "geos/capi/*.cpp")',
				'<!@(python ../glob-files.py "geos/src/*.cpp")',
				'<!@(python ../glob-files.py "geos/src/*/*.cpp")',
				'<!@(python ../glob-files.py "geos/src/*/*/*.cpp")',
				'<!@(python ../glob-files.py "geos/src/*/*/*/*.cpp")'
			],
			"include_dirs": [
				"./arch/common",
				"./geos/include",
				"./geos/capi"
			],
			"defines": [
				"USE_UNSTABLE_GEOS_CPP_API"
			],
			"conditions": [
				["OS == 'win'", {
					"include_dirs": ["./arch/win"],
					"defines": [ "NOMINMAX" ]
				}, {
					"include_dirs": ["./arch/unix"]
				}]
			],
			"libraries": ["<!@(geos-config --libs)"],
			"link_settings": {
				"ldflags": [
					"<!@(geos-config --ldflags)",
					"-lgeos_c"
				]
			},
			"cflags_cc!": ["<!@(geos-config --cflags)"],
			"cflags!": ["<!@(geos-config --cflags)"],
			"xcode_settings": {
				"GCC_ENABLE_CPP_RTTI": "YES",
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
			},
			"direct_dependent_settings": {
				"include_dirs": [
					"./arch/common",
					"./geos/include",
					"./geos/capi"
				],
				"conditions": [
					["OS == 'win'", {
						"include_dirs": ["./arch/win"]
					}, {
						"include_dirs": ["./arch/unix"]
					}]
				],
			}
		}
	]
}
