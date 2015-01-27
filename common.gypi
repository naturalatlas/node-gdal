{
	"variables": {
		"toolset%":'',
		"prefers_libcpp": "<!(python -c \"import os;import platform;u=platform.uname();print((u[0] == 'Darwin' and int(u[2][0:2]) >= 13) and '-stdlib=libstdc++' not in os.environ.get('CXXFLAGS','') and '-mmacosx-version-min' not in os.environ.get('CXXFLAGS',''))\")"
	},
	"target_defaults": {
		"default_configuration": "Release",
		"msbuild_toolset":"<(toolset)",
		"cflags_cc!": ["-fno-exceptions"],
		"cflags!": ["-fno-exceptions"],
		"defines": [
			"NOGDI=1"
		],
		"xcode_settings": {
			"GCC_ENABLE_CPP_EXCEPTIONS": "YES",
			"CLANG_CXX_LANGUAGE_STANDARD": "c++11",
			"OTHER_CFLAGS": [
				"-mmacosx-version-min=10.7",
				"-Wno-deprecated-register",
				"-Wno-unused-const-variable"
			],
			"OTHER_CPLUSPLUSFLAGS": [
				"-mmacosx-version-min=10.7",
				"-std=c++11",
				"-stdlib=libc++",
				"-Wno-deprecated-register",
				"-Wno-unused-const-variable"
			]
		},
		"conditions": [
			["'<(prefers_libcpp)' == 'True'", {
				"xcode_settings": {
					"MACOSX_DEPLOYMENT_TARGET": "10.9"
				}
			}]
		],
		"configurations": {
			"Debug": {
				"cflags_cc!": ["-O3", "-Os", "-DNDEBUG"],
				"xcode_settings": {
					"OTHER_CPLUSPLUSFLAGS": ["-DDEBUG"],
					"GCC_OPTIMIZATION_LEVEL": "0",
					"GCC_GENERATE_DEBUGGING_SYMBOLS": "YES"
				},
				"msvs_settings": {
					"VCCLCompilerTool": {
						"ExceptionHandling": 2, # /EHsc
						"RuntimeTypeInfo": "true"
					}
				}
			},
			"Release": {
				'defines': [ 'NDEBUG' ],
				"xcode_settings": {
					"GCC_OPTIMIZATION_LEVEL": "s",
					"GCC_GENERATE_DEBUGGING_SYMBOLS": "NO",
					"DEAD_CODE_STRIPPING": "YES",
					"GCC_INLINES_ARE_PRIVATE_EXTERN": "YES"
				},
				"ldflags": [
					"-Wl,-s"
				],
				"msvs_settings": {
					"VCCLCompilerTool": {
						"ExceptionHandling": 2, # /EHsc
						"RuntimeTypeInfo": "true",
						"DebugInformationFormat": "0"
					},
					"VCLinkerTool": {
						"GenerateDebugInformation": "false",
					}
				}
			}
		}
	}
}