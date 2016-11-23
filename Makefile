CXXFLAGS := -fPIC -O2

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

clean:
	rm *.o *.so Compiler Renderer
