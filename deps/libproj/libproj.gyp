{
	"includes": [
		"../../common.gypi"
	],
	"targets": [
		{
			"target_name": "libproj",
			"type": "static_library",
			"sources": [
				"proj/src/geodesic.c",
				"proj/src/aasincos.c",
				"proj/src/adjlon.c",
				"proj/src/bch2bps.c",
				"proj/src/bchgen.c",
				"proj/src/biveval.c",
				# "proj/src/cs2cs.c",
				"proj/src/dmstor.c",
				"proj/src/emess.c",
				# "proj/src/gen_cheb.c",
				"proj/src/geocent.c",
				"proj/src/geod.c",
				"proj/src/geod_set.c",
				# "proj/src/jniproj.c",
				"proj/src/mk_cheby.c",
				"proj/src/nad_cvt.c",
				"proj/src/nad_init.c",
				"proj/src/nad_intr.c",
				# "proj/src/p_series.c",
				"proj/src/rtodms.c",
				"proj/src/vector1.c",
				"proj/src/pj_fileapi.c",
				'<!@(python ../glob-files.py "proj/src/PJ_*.c")',
				'<!@(python ../glob-files.py "proj/src/proj_*.c")',
				'<!@(python ../glob-files.py "proj/src/pj_*.c")'
			],
			"include_dirs": [
				"./proj/src"
			],
			"conditions": [
				["OS == 'win'", {
					"include_dirs": ["./arch/win/src"]
				}, {
					"include_dirs": ["./arch/unix/src"]
				}]
			],
			"cflags_cc!": ["-fno-rtti", "-fno-exceptions"],
			"cflags!": ["-fno-rtti", "-fno-exceptions"],
			"xcode_settings": {
				"GCC_ENABLE_CPP_RTTI": "YES",
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
			},
			"direct_dependent_settings": {
				"include_dirs": [
					"./proj/src"
				],
				"defines": [],
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
