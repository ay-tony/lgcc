all:
	antlr4 -Dlanguage=Cpp c.g4 -o antlr_generate/; \
	g++ antlr_generate/*.cpp main.cpp -o lgcc -I/usr/include/antlr4-runtime/ -Iantlr_generate/ -lantlr4-runtime

clean:
	rm -r antlr_generate/ lgcc
