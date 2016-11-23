CXXFLAGS := -fPIC -O2

all:
	make SimpleMark
	make SimpleMark-Compiler
	make SimpleMark-Renderer

SimpleMark: Parser.o
	$(CXX) -shared -o libSimpleMark.so Parser.o

SimpleMark-Compiler: Compiler.o
	$(CXX) -o Compiler Compiler.o -L. -lSimpleMark

SimpleMark-Renderer: Renderer.o
	$(CXX) -o Renderer Renderer.o -L. -lSimpleMark

clean:
	rm *.o *.so Compiler Renderer
