/* Copyright (c) 2012-2017 The ANTLR Project. All rights reserved.
 * Use of this file is governed by the BSD 3-clause license that
 * can be found in the LICENSE.txt file in the project root.
 */

//
//  main.cpp
//  antlr4-cpp-demo
//
//  Created by Mike Lischke on 13.03.16.
//

#include <cstdio>
#include <iostream>

#include "lgccLexer.h"
#include "lgccParser.h"
#include <antlr4-runtime.h>

using namespace antlrcpp;
using namespace antlr4;

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "Error: no input files!\n");
    return 1;
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

  std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

  return 0;
}