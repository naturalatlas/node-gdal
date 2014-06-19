{
	"includes": [
		"../common.gypi"
	],
	"targets": [
		{
			"target_name": $TARGET_NAME,
			"type": "static_library",
			"sources": [
				$SOURCES
			],
			"include_dirs": [
				$INCLUDE_DIRS
			]
		}
	]
}
