CXXFLAGS := -fPIC -std=c++11 -O2

all:
	make OxygenMark
	make OxygenMark-Compiler
	make OxygenMark-Renderer

OxygenMark: Parser.o
	$(CXX) -shared -o libOxygenMark.so Parser.o

OxygenMark-Compiler: Compiler.o
	$(CXX) -o Compiler Compiler.o -L. -lOxygenMark

OxygenMark-Renderer: Renderer.o
	$(CXX) -shared -o libOxygenMarkRenderer.so Renderer.o -L. -lOxygenMark

OxygenMark-Bundle-Emscripten:
	em++ -std=c++11 -O1 -o OxygenMarkBundle.js Parser.cpp Renderer.cpp -s "EXPORTED_FUNCTIONS=['_loadDocumentFromSource', '_setDocumentParam', '_renderToHtml']" -s "DISABLE_EXCEPTION_CATCHING=0"
	uglifyjs -o OxygenMarkBundle.min.js OxygenMarkBundle.js
	rm OxygenMarkBundle.js

clean:
	rm *.o *.so Compiler OxygenMarkBundle.min.js
