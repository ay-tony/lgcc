#include <antlr4-runtime.h>
#include <cstdio>
#include <iostream>

#include "lgccLexer.h"
#include "lgccParser.h"
#include "visitor.h"

using namespace antlrcpp;
using namespace antlr4;

int main(int argc, const char *argv[]) {
  // 参数处理
  if (argc == 1) {
    fprintf(stderr, "Error: no input files!\n");
    return -1;
  }

  for (int i = 1; i < argc; i++)
    if (argv[i][0] != '-')
      freopen(argv[i], "r", stdin);
    else
      freopen(argv[++i], "w", stdout);

  ANTLRInputStream input(std::cin);
  lgccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  lgccParser parser(&tokens);
  tree::ParseTree *tree = parser.program();
  visitor visitor;
  visitor.visit(tree);

  return lexer.getNumberOfSyntaxErrors() + parser.getNumberOfSyntaxErrors();
}