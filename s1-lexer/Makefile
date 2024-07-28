all: lexer.l sample.c
	flex lexer.l; \
  	g++ -o lexer lex.yy.cc -Wall -Wextra; \
    ./lexer < sample.c