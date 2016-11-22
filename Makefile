CXXFLAGS := -fPIC -O2

SimpleMark: Parser.o
	$(CXX) -shared -o libSimpleMark.so Parser.o

SimpleMark-Compiler: Compiler.o
	$(CXX) -o Compiler Compiler.o -L. -lSimpleMark

clean:
	rm *.o *.so
