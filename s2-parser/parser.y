%{
#define YYSTYPE char*
#include "lex.yy.h"

void yyerror(const char *s);

int indent = 0;
%}

%token T_TYPE
%token T_RETURN
%token T_IDENTIFIER
%token T_INTEGER_LITERAL

%%

CompUnit
: FuncDef { }

FuncDef
: T_TYPE T_IDENTIFIER '(' ')' { printf("define %s @%s() ", $1, $2); } Block

Block
: '{' { printf("{\n"); indent++; } Statement '}' { printf("}\n"); indent--; }

Statement
: T_RETURN T_INTEGER_LITERAL ';'
  { for(int i = 0; i < indent * 4; i++) putchar(' '); printf("ret i32 %s\n", $2); }

%%

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

