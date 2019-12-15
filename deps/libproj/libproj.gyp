{
	"includes": [
		"../../common.gypi"
	],
	"targets": [
		{
			"target_name": "libproj",
			"type": "static_library",
			"sources": [
				'<!@(python ../glob-files.py "proj/src/*.c")',
				'<!@(python ../glob-files.py "proj/src/*.cpp")',
				'<!@(python ../glob-files.py "proj/src/*/*.cpp")'
			],
			"include_dirs": [
				"./proj/src",
				"./proj/include"
			],
			"conditions": [
				["OS == 'win'", {
					"include_dirs": ["./arch/win/src"]
				}, {
					"include_dirs": ["./arch/unix/src"]
				}]
			],
			"dependencies": [
				"../libsqlite/libsqlite.gyp:libsqlite"
			],
			"defines": [
			  'PROJ_LIB="<(module_path)/../../../deps/libproj/proj/data"'
			],
			"xcode_settings": {
				"GCC_ENABLE_CPP_RTTI": "YES",
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
			},
			"direct_dependent_settings": {
				"include_dirs": [
					"./proj/src"
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
