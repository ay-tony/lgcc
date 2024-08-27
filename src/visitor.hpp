#ifndef VISITOR_H
#define VISITOR_H

#include <any>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ostream>
#include <string>

#include <Parser.h>
#include <tree/TerminalNode.h>

#include "lgccBaseVisitor.h"
#include "lgccParser.h"
#include "symbol.hpp"

// TODO: 将定义移到 .cpp 文件中

class visitor_t : public lgccBaseVisitor {
private:
  size_t m_indent{};
  std::ofstream m_outfile;
  std::unique_ptr<scope_t> m_current_scope{new scope_t};
  std::uint32_t m_ir_cnt{1};

  void pd() {
    for (size_t i = 0; i < m_indent; i++)
      m_outfile << "    ";
  }

  template <class... Args>
  void p(std::format_string<Args...> fmt, Args &&...args) {
    m_outfile << std::format(fmt, args...);
  }

  template <class... Args>
  void pl(std::format_string<Args...> fmt, Args &&...args) {
    pd();
    p(fmt, args...);
    std::endl(m_outfile);
  }

public:
  visitor_t(const std::string &outf) {
    m_outfile.open(outf, std::ios::out | std::ios::trunc);
  }

  ~visitor_t() { m_outfile.close(); }

  std::any visitProgram(lgccParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any visitFunction_definition(
      lgccParser::Function_definitionContext *ctx) override {
    std::string typ{std::any_cast<std::string>(visit(ctx->function_type()))};
    std::string id{std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    pd();
    p("define {} @{}() ", typ, id);
    visit(ctx->block());
    return defaultResult();
  }

  std::any
  visitFunctionTypeInt(lgccParser::FunctionTypeIntContext *ctx) override {
    return std::string{"i32"};
  }

  std::any visitBlock(lgccParser::BlockContext *ctx) override {
    p("{{\n");
    m_indent++;
    visitChildren(ctx);
    m_indent--;
    p("}}\n");
    return defaultResult();
  }

  virtual std::any visitReturnExpressionStatement(
      lgccParser::ReturnExpressionStatementContext *ctx) override {
    auto val = std::any_cast<std::uint32_t>(visit(ctx->expression()));
    pl("ret i32 %{}", val);
    return defaultResult();
  }

  virtual std::any visitReturnConstExpressionStatement(
      lgccParser::ReturnConstExpressionStatementContext *ctx) override {
    auto val = std::any_cast<int32_t>(visit(ctx->const_expression()));
    pl("ret i32 {}", val);
    return defaultResult();
  }

  std::any
  visitVariableTypeInt(lgccParser::VariableTypeIntContext *ctx) override {
    return std::string{"i32"};
  }

  std::any visitNoInitializeVariableDefinition(
      lgccParser::NoInitializeVariableDefinitionContext *ctx) override {
    std::string symbol_name{
        std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    auto symbol_ir_cnt = m_ir_cnt;
    m_current_scope->insert_variable(symbol_name, variable_t(m_ir_cnt++));
    pl("%{} = alloca i32", symbol_ir_cnt);
    return defaultResult();
  }

  std::any visitConstExpressionInitializeVariableDefinition(
      lgccParser::ConstExpressionInitializeVariableDefinitionContext *ctx)
      override {
    std::string symbol_name{
        std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    auto val = std::any_cast<int32_t>(visit(ctx->const_expression()));
    auto symbol_ir_cnt = m_ir_cnt;
    m_current_scope->insert_variable(symbol_name, variable_t(m_ir_cnt++));
    pl("%{} = alloca i32", symbol_ir_cnt);
    pl("store i32 {}, ptr %{}", val, symbol_ir_cnt);
    return defaultResult();
  }

  std::any visitExpressionInitializeVariableDefinition(
      lgccParser::ExpressionInitializeVariableDefinitionContext *ctx) override {
    std::string symbol_name{
        std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    auto val = std::any_cast<uint32_t>(visit(ctx->expression()));
    auto symbol_ir_cnt = m_ir_cnt;
    m_current_scope->insert_variable(symbol_name, variable_t(m_ir_cnt++));
    pl("%{} = alloca i32", symbol_ir_cnt);
    pl("store i32 %{}, ptr %{}", val, symbol_ir_cnt);
    return defaultResult();
  }

  virtual std::any visitUnaryConstExpression(
      lgccParser::UnaryConstExpressionContext *ctx) override {
    if (ctx->op->getText() == "+")
      return std::any_cast<int32_t>(visit(ctx->const_expression()));
    else
      return -std::any_cast<int32_t>(visit(ctx->const_expression()));
  }

  virtual std::any visitBraceConstExpression(
      lgccParser::BraceConstExpressionContext *ctx) override {
    return visit(ctx->const_expression());
  }

  virtual std::any visitIntegerConstExpression(
      lgccParser::IntegerConstExpressionContext *ctx) override {
    return visit(ctx->LITERAL_INTEGER());
  }

  virtual std::any visitBinaryConstExpression(
      lgccParser::BinaryConstExpressionContext *ctx) override {
    switch (ctx->op->getText()[0]) {
    case '+':
      return std::any_cast<int32_t>(visit(ctx->lhs)) +
             std::any_cast<int32_t>(visit(ctx->rhs));
    case '-':
      return std::any_cast<int32_t>(visit(ctx->lhs)) -
             std::any_cast<int32_t>(visit(ctx->rhs));
    case '*':
      return std::any_cast<int32_t>(visit(ctx->lhs)) *
             std::any_cast<int32_t>(visit(ctx->rhs));
    case '/':
      return std::any_cast<int32_t>(visit(ctx->lhs)) /
             std::any_cast<int32_t>(visit(ctx->rhs));
    default:
      return std::any_cast<int32_t>(visit(ctx->lhs)) %
             std::any_cast<int32_t>(visit(ctx->rhs));
    }
  }

  virtual std::any
  visitUnaryExpression(lgccParser::UnaryExpressionContext *ctx) override {
    if (ctx->op->getText() == "+")
      return visit(ctx->expression());
    else {
      auto child_ir_cnt = std::any_cast<uint32_t>(visit(ctx->expression()));
      auto cur_ir_cnt = m_ir_cnt++;
      pl("%{} = mul i32 -1, %{}", cur_ir_cnt, child_ir_cnt);
      return cur_ir_cnt;
    }
  }

  virtual std::any
  visitBinaryExpression(lgccParser::BinaryExpressionContext *ctx) override {
    auto child1_ir_cnt = std::any_cast<uint32_t>(visit(ctx->lhs));
    auto child2_ir_cnt = std::any_cast<uint32_t>(visit(ctx->rhs));
    auto cur_ir_cnt = m_ir_cnt++;
    switch (ctx->op->getText()[0]) {
    case '+':
      pl("%{} = add i32 %{}, %{}", cur_ir_cnt, child1_ir_cnt, child2_ir_cnt);
      break;
    case '-':
      pl("%{} = sub i32 %{}, %{}", cur_ir_cnt, child1_ir_cnt, child2_ir_cnt);
      break;
    case '*':
      pl("%{} = mul i32 %{}, %{}", cur_ir_cnt, child1_ir_cnt, child2_ir_cnt);
      break;
    case '/':
      pl("%{} = sdiv i32 %{}, %{}", cur_ir_cnt, child1_ir_cnt, child2_ir_cnt);
      break;
    case '%':
      pl("%{} = srem i32 %{}, %{}", cur_ir_cnt, child1_ir_cnt, child2_ir_cnt);
      break;
    default:
      throw; // TODO: 规范化错误处理
    }
    return cur_ir_cnt;
  }

  virtual std::any
  visitBraceExpression(lgccParser::BraceExpressionContext *ctx) override {
    return visit(ctx->expression());
  }

  virtual std::any visitConstExpressionExpression(
      lgccParser::ConstExpressionExpressionContext *ctx) override {
    auto child_val = std::any_cast<int32_t>(visit(ctx->const_expression()));
    auto cur_ir_cnt = m_ir_cnt++;
    pl("%{} = {}", cur_ir_cnt, child_val);
    return cur_ir_cnt;
  }

  virtual std::any visitIdentifierExpression(
      lgccParser::IdentifierExpressionContext *ctx) override {
    auto symbol_ir_cnt = m_current_scope
                             ->resolve_variable(std::any_cast<std::string>(
                                 visit(ctx->IDENTIFIER())))
                             .get_ir_cnt();
    auto cur_ir_cnt = m_ir_cnt++;
    pl("%{} = load i32, ptr %{}", cur_ir_cnt, symbol_ir_cnt);
    return cur_ir_cnt;
  }

  std::any visitTerminal(antlr4::tree::TerminalNode *ctx) override {
    switch (ctx->getSymbol()->getType()) {
    case lgccParser::LITERAL_INTEGER:
      return static_cast<int32_t>(std::stoi(ctx->getText(), nullptr, 0));
    default: // lgccParser::IDENTIFIER
      return ctx->getText();
    }
  }
};

#endif // VISITOR_H
