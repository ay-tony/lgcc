#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TOKEN {
    T_END_OF_FILE,  // EOF must be 0 to suit lexer
    T_KEYWORD,
    T_IDENTIFIER,
    T_SEMICOLON,
    T_BRACKET_LEFT_OPEN,
    T_BRACKET_RIGHT_OPEN,
    T_BRACKET_LEFT_CURLY,
    T_BRACKET_RIGHT_CURLY,
    T_INTEGER_LITERAL
};

static void print_token(int token) {
    std::string token_strs[] = {
        "",
        "keyword",
        "indetifier",
        ";",
        "(",
        ")",
        "{",
        "}",
        "integer literal",
        "<<EOF>>"
    };

    printf("%-20s", token_strs[static_cast<size_t>(token)].c_str());
}

#endif  // TOKEN_HPP