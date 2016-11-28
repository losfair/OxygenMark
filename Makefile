CXXFLAGS := -fPIC -std=c++11 -O2

all:
	make OxygenMark
	make OxygenMark-Compiler
	make OxygenMark-Renderer

OxygenMark: Tokenizer.o NewParser.o
	$(CXX) -shared -o libOxygenMark.so Tokenizer.o NewParser.o

OxygenMark-Compiler: Compiler.o
	$(CXX) -o Compiler Compiler.o -L. -lOxygenMark

OxygenMark-Renderer: Renderer.o
	$(CXX) -shared -o libOxygenMarkRenderer.so Renderer.o -L. -lOxygenMark

OxygenMark-Bundle-Emscripten:
	em++ -std=c++11 -O3 -o OxygenMarkBundle.js Tokenizer.cpp NewParser.cpp Renderer.cpp -s "EXPORTED_FUNCTIONS=['_loadDocumentFromSource', '_setDocumentParam', '_clearDocumentParams', '_renderToHtml', '_destroyDocument']" -s "DISABLE_EXCEPTION_CATCHING=0"
	uglifyjs -o OxygenMarkBundle.min.js OxygenMarkBundle.js
	rm OxygenMarkBundle.js

clean:
	rm *.o *.so *.js.mem Compiler OxygenMarkBundle.min.js
