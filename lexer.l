%option noyywrap

%{
#define YYSTYPE char*
#include "y.tab.h"

void lex_error(const char* msg, int line);

size_t cur_line = 1;
%}

%%
"int"               { yylval = strdup("i32"); return T_TYPE; }

"return"            { return T_RETURN; }

"main"              { yylval = strdup(yytext); return T_IDENTIFIER; }

[{}();]             { return yytext[0]; }

[0-9]+              { yylval = strdup(yytext); return T_INTEGER_LITERAL; }

\/\/.*              { }
\/\*[/]*([^*/][/]*|[*]|[^*/])*\*\/ { }

[\n]                { cur_line++; }
[ \t\r\a]+          { }
<<EOF>>             { return 0; }

.                   { lex_error("Unrecognized character", cur_line); }
%%

void lex_error(const char* msg, int line) {
    printf("\nError at line %-3d: %s\n\n", line, msg);
}