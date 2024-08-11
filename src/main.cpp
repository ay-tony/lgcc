#include <cstdio>
#include <fstream>
#include <iostream>

#include <antlr4-runtime/antlr4-runtime.h>
#include <argparse/argparse.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "lgccLexer.h"
#include "lgccParser.h"
#include "visitor.h"

class lgcc_error_listener : public antlr4::BaseErrorListener {
  void syntaxError(antlr4::Recognizer *recognizer,
                   antlr4::Token *offendingSymbol, size_t line,
                   size_t charPositionInLine, const std::string &msg,
                   std::exception_ptr e) override {
    spdlog::error(":{}:{}: {}", line, charPositionInLine, msg);
  }
};

// TODO: 支持 lgcc 日志功能
int main(int argc, char *argv[]) {
  // 定义参数
  // TODO: 从 xmake 配置读取并写到这里，修改默认参数
  argparse::ArgumentParser program("lgcc", "0.1.0");
  program.add_argument("-o", "--output")
      .help("the file to generate")
      .default_value("a.out");
  program.add_argument("files");

  // 用 argparse 解析参数
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    spdlog::critical("error passing arguments: {}", err.what());
    spdlog::critical("exiting lgcc.");
    std::exit(1);
  }

  // 参数提取
  // TODO: 支持多文件输入
  auto source_file{program.get<std::vector<std::string>>("files")[0]};
  auto out_file{program.get<std::string>("--output")};

  // 开始解析过程
  // 读取源文件并转化成输入流
  std::ifstream source_file_stream;
  source_file_stream.open(source_file, std::ios::in);
  antlr4::ANTLRInputStream input(source_file_stream);
  source_file_stream.close();

  // 词法分析
  lgccLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  // 语法分析
  lgccParser parser(&tokens);
  parser.removeErrorListeners();
  lgcc_error_listener error_listener;
  parser.addErrorListener(&error_listener);
  antlr4::tree::ParseTree *tree = parser.program();
  visitor visitor(out_file);
  visitor.visit(tree);

  return lexer.getNumberOfSyntaxErrors() + parser.getNumberOfSyntaxErrors();
}
