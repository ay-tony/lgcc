all: lexer.l parser.y
	bison -vdty parser.y; \
	flex --header-file=lex.yy.h lexer.l; \
  	gcc -o lgcc y.tab.c lex.yy.c main.c -Wall -Wextra -Wno-free-nonheap-object -Wno-unused-function; \
	./lgcc < sample.c | tee sample.ll; \
	lli ./sample.ll

clean:
	rm lex.yy.c lex.yy.h lgcc sample.ll y.output y.tab.c y.tab.h