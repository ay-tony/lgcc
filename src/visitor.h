#ifndef VISITOR_H
#define VISITOR_H

#include "lgccBaseVisitor.h"
#include "lgccParser.h"
#include <Parser.h>
#include <any>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ostream>
#include <string>
#include <tree/TerminalNode.h>

// TODO: 将定义移到 .cpp 文件中

class visitor : public lgccBaseVisitor {
private:
  size_t m_indent = 0;

  void pd() {
    for (size_t i = 0; i < m_indent; i++)
      std::cout << "    ";
  }

  template <class... Args>
  void p(std::format_string<Args...> fmt, Args &&...args) {
    std::cout << std::format(fmt, args...);
  }

  template <class... Args>
  void pl(std::format_string<Args...> fmt, Args &&...args) {
    pd();
    p(fmt, args...);
    std::endl(std::cout);
  }

public:
  std::any visitProgram(lgccParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any visitFunction_definition(
      lgccParser::Function_definitionContext *ctx) override {
    std::string typ{std::any_cast<std::string>(visit(ctx->function_type()))};
    std::string id{
        std::any_cast<std::string>(visitTerminal(ctx->IDENTIFIER()))};
    pd();
    p("define {} @{}() ", typ, id);
    visit(ctx->block());
    return defaultResult();
  }

  std::any visitKeyInt(lgccParser::KeyIntContext *ctx) override {
    return std::string{"i32"};
  }

  std::any visitStatements(lgccParser::StatementsContext *ctx) override {
    p("{{\n");
    m_indent++;
    visit(ctx->statement());
    m_indent--;
    p("}}\n");
    return defaultResult();
  }

  std::any
  visitReturnStatement(lgccParser::ReturnStatementContext *ctx) override {
    auto val = std::any_cast<std::int32_t>(visit(ctx->LITERAL_INTEGER()));
    pl("ret i32 {}", val);
    return defaultResult();
  }

  std::any
  visitBinaryExpression(lgccParser::BinaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any
  visitUnaryExpression(lgccParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any
  visitIntegerExpression(lgccParser::IntegerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any
  visitBraceExpression(lgccParser::BraceExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any visitTerminal(antlr4::tree::TerminalNode *ctx) override {
    switch (ctx->getSymbol()->getType()) {
    case lgccParser::LITERAL_INTEGER:
      return static_cast<int32_t>(std::stoi(ctx->getText(), nullptr, 0));
    default:
      return ctx->getText();
    }
  }
};

#endif // VISITOR_H
