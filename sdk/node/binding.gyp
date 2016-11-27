{
	"targets": [
		{
			"target_name": "OxygenMark",
			"sources": [
				"Tokenizer.cpp",
				"NewParser.cpp",
				"Renderer.cpp",
				"NodeIntegration.cpp"
			],
			'cflags!': [ '-fno-exceptions' ],
			'cflags_cc!': [ '-fno-exceptions' ]
		}
	]
}
