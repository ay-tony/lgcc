%option noyywrap

%{
#include <stdint.h>
#include "y.tab.h"

void lex_error(const char* msg, int line);

size_t cur_line = 1;
int32_t atoi_o(const char *);
int32_t atoi_h(const char *);
%}

%%

"return"            { return T_KEY_RETURN; }
"int"               { return T_KEY_INT; }
"main"              { yylval.name = strdup(yytext); return T_IDENTIFIER; }
[1-9][0-9]+         { yylval.inum = atoi(yytext); return T_INTEGER_LITERAL; }
0[0-9]+             { yylval.inum = atoi_o(yytext); return T_INTEGER_LITERAL; }
0[xX][0-9a-fA-F]+   { yylval.inum = atoi_h(yytext); return T_INTEGER_LITERAL; }

\/\/.*              { }
\/\*[/]*([^*/][/]*|[*]|[^*/])*\*\/ { }

[{}()+\-*/;]        { return yytext[0]; }

[\n]                { cur_line++; }
[ \t\r\a]+          { }
<<EOF>>             { return 0; }

.                   { lex_error("Unrecognized character", cur_line); }
%%

void lex_error(const char* msg, int line) {
    printf("\nError at line %-3d: %s\n\n", line, msg);
}

int32_t atoi_o(const char *s) {
    int32_t ret = 0;
    for(const char *c = s + 1; *c; c++)
        ret = ret * 8 + *c - '0';
    return ret;
}

int32_t atoi_h(const char *s) {
    int32_t ret = 0;
    for(const char *c = s + 2; *c; c++) {
        ret = ret * 16;
        if (*c >= 'a' && *c <= 'f') ret += *c - 'a' + 10;
        else if (*c >= 'A' && *c <= 'F') ret += *c - 'A' + 10;
        else ret += *c - '0';
    }
    return ret;
}