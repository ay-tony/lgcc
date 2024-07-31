all: lexer.l parser.y
	bison -vdty parser.y; \
	flex --header-file=lex.yy.h lexer.l; \
  	g++ -o lgcc y.tab.c lex.yy.c main.c -Wall -Wextra -Wno-free-nonheap-object -Wno-unused-function; \
	./lgcc < sample.c | tee sample.ll