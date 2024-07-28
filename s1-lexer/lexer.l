%option c++
%option noyywrap

%{
#include "token.hpp"

void init_scanner();
void lex_error(char* msg, int line);

size_t cur_line = 1;
%}

%%

"int"               { return T_KEYWORD; }
"return"            { return T_KEYWORD; }
"main"              { return T_IDENTIFIER; }
";"                 { return T_SEMICOLON; }
"("                 { return T_BRACKET_LEFT_OPEN; }
")"                 { return T_BRACKET_LEFT_OPEN; }
"{"                 { return T_BRACKET_LEFT_CURLY; }
"}"                 { return T_BRACKET_LEFT_CURLY; }
[0-9]+              { return T_INTEGER_LITERAL; }
<<EOF>>             { return T_END_OF_FILE; }

[\n]                { cur_line++; }
[ \t\r\a]+          { /* ignore all spaces */ }

.                   { char sz[] = "Unrecognized character"; lex_error(sz, cur_line); }

%%

int main() {
    init_scanner();

    FlexLexer* lexer = new yyFlexLexer();
    while (int token = lexer->yylex()) {
        print_token(token);
        puts(lexer->YYText());
    }
    return 0;
}

void init_scanner() {
    printf("%-20s%s\n", "TOKEN-TYPE", "TOKEN-VALUE");
    printf("-------------------------------------------------\n");
}

void lex_error(char* msg, int line) {
    printf("\nError at line %-3d: %s\n\n", line, msg);
}