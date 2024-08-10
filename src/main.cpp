#include <cstdio>
#include <iostream>

#include <antlr4-runtime.h>
#include <argparse/argparse.hpp>

#include "lgccLexer.h"
#include "lgccParser.h"
#include "visitor.h"

int main(int argc, char *argv[]) {
  // TODO: 从 xmake 配置读取并写到这里，修改默认参数
  argparse::ArgumentParser program("lgcc", "0.1.0");
  program.add_argument("-o", "--output")
      .help("the file to generate")
      .default_value("a.out");
  program.add_argument("source_files");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  // TODO: 用 c++ 的输入输出流重写
  freopen(program.get<std::string>("--output").c_str(), "w", stdout);
  freopen(program.get<std::vector<std::string>>("source_files")[0].c_str(), "r",
          stdin);

  antlr4::ANTLRInputStream input(std::cin);
  lgccLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  lgccParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.program();
  visitor visitor;
  visitor.visit(tree);

  return lexer.getNumberOfSyntaxErrors() + parser.getNumberOfSyntaxErrors();
}
