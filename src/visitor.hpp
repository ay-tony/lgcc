#ifndef VISITOR_HPP
#define VISITOR_HPP

#include <any>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ostream>
#include <string>
#include <typeinfo>

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
  std::optional<std::string> m_current_function_name;
  variable_t::ir_cnt_t m_ir_cnt{1};

  void pd() {
    for (size_t i = 0; i < m_indent; i++)
      m_outfile << "    ";
  }

  template <class... Args> void p(std::format_string<Args...> fmt, Args &&...args) { m_outfile << std::format(fmt, args...); }

  template <class... Args> void pl(std::format_string<Args...> fmt, Args &&...args) {
    pd();
    p(fmt, args...);
    std::endl(m_outfile);
  }

  struct const_expression_t {
    std::any val;
    variable_t::TYPE type;
  };

  struct expression_t {
    variable_t::ir_cnt_t ir_cnt;
    variable_t::TYPE type;
  };

  std::string to_string(std::any &val) {
    if (val.type() == typeid(std::int32_t))
      return std::to_string(std::any_cast<std::int32_t>(val));
    else if (val.type() == typeid(float))
      return std::to_string(std::any_cast<float>(val));
    throw; // TODO
  }

  void expression_conversion(variable_t::TYPE from_type, variable_t::ir_cnt_t from_ir_cnt, variable_t::TYPE to_type,
                             variable_t::ir_cnt_t to_ir_cnt) {
    auto from_type_name = variable_t::to_string(from_type);
    auto to_type_name = variable_t::to_string(to_type);
    switch (from_type) {
    case variable_t::TYPE::INT32:
      switch (to_type) {
      case variable_t::TYPE::INT32:
        pl("%{} = %{}", to_ir_cnt, from_ir_cnt);
        return;
      case variable_t::TYPE::FLOAT:
        pl("%{} = sitofp {} %{} to {}", to_ir_cnt, from_type_name, from_ir_cnt, to_type_name);
        return;
      }
    case variable_t::TYPE::FLOAT:
      switch (to_type) {
      case variable_t::TYPE::INT32:
        pl("%{} = fptosi {} %{} to {}", to_ir_cnt, from_type_name, from_ir_cnt, to_type_name);
        return;
      case variable_t::TYPE::FLOAT:
        pl("%{} = %{}", to_ir_cnt, from_ir_cnt);
        return;
      }
    }
  }

  std::any const_expression_conversion(variable_t::TYPE from_type, std::any from_val, variable_t::TYPE to_type) {
    switch (from_type) {
    case variable_t::TYPE::INT32:
      switch (to_type) {
      case variable_t::TYPE::INT32:
        return from_val;
      case variable_t::TYPE::FLOAT:
        return static_cast<float>(std::any_cast<std::int32_t>(from_val));
      }
    case variable_t::TYPE::FLOAT:
      switch (to_type) {
      case variable_t::TYPE::INT32:
        return static_cast<std::int32_t>(std::any_cast<float>(from_val));
      case variable_t::TYPE::FLOAT:
        return from_val;
      }
    }
  }

  variable_t::TYPE get_common_type(variable_t::TYPE type1, variable_t::TYPE type2) {
    if (type1 == variable_t::TYPE::FLOAT || type2 == variable_t::TYPE::FLOAT)
      return variable_t::TYPE::FLOAT;
    return variable_t::TYPE::INT32;
  }

public:
  visitor_t(const std::string &outf) { m_outfile.open(outf, std::ios::out | std::ios::trunc); }

  ~visitor_t() { m_outfile.close(); }

  std::any visitProgram(lgccParser::ProgramContext *ctx) override { return visitChildren(ctx); }

  std::any visitFunction_definition(lgccParser::Function_definitionContext *ctx) override {
    auto raw_type_name{std::any_cast<std::string>(visit(ctx->function_type()))};
    auto type{function_t::to_type(raw_type_name)};
    auto type_name{function_t::to_string(type)};
    auto id{std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};

    m_current_function_name = id;
    m_current_scope->insert_function(id, function_t(type));

    pd();
    p("define {} @{}() ", type_name, id);
    visit(ctx->block());

    m_current_function_name.reset();
    return defaultResult();
  }

  std::any visitBlock(lgccParser::BlockContext *ctx) override {
    // TODO: 添加符号表域
    p("{{\n");
    m_indent++;
    visitChildren(ctx);
    m_indent--;
    p("}}\n");
    return defaultResult();
  }

  std::any visitReturnExpressionStatement(lgccParser::ReturnExpressionStatementContext *ctx) override {
    auto [ir_cnt, type]{std::any_cast<expression_t>(visit(ctx->expression()))};
    auto return_type{m_current_scope->resolve_function(m_current_function_name.value()).get_return_type()};
    if (return_type != function_t::TYPE::VOID) {
      auto variable_type = function_t::to_variable_type(return_type);
      if (variable_type != type) {
        auto converted_ir_cnt = m_ir_cnt++;
        expression_conversion(type, ir_cnt, variable_type, converted_ir_cnt);
        ir_cnt = converted_ir_cnt;
      }
      auto variable_type_name{variable_t::to_string(variable_type)};
      pl("ret {} %{}", variable_type_name, ir_cnt);
    }
    return defaultResult();
  }

  std::any visitReturnConstExpressionStatement(lgccParser::ReturnConstExpressionStatementContext *ctx) override {
    auto [val, type]{std::any_cast<const_expression_t>(visit(ctx->const_expression()))};
    auto return_type{m_current_scope->resolve_function(m_current_function_name.value()).get_return_type()};
    if (return_type != function_t::TYPE::VOID) {
      auto cur_type = function_t::to_variable_type(return_type);
      if (cur_type != type)
        val = const_expression_conversion(type, val, cur_type);
      auto variable_type_name{variable_t::to_string(cur_type)};
      auto val_str = to_string(val);
      pl("ret {} {}", variable_type_name, val_str);
    }
    return defaultResult();
  }

  std::any visitVariable_definition_statement(lgccParser::Variable_definition_statementContext *ctx) override {
    auto raw_type_name = std::any_cast<std::string>(visit(ctx->variable_type()));
    auto type = variable_t::to_type(raw_type_name);
    auto type_name = variable_t::to_string(type);

    for (auto child : ctx->single_variable_definition()) {
      auto [name, val] = std::any_cast<std::tuple<std::string, std::any>>(visit(child));

      auto ir_cnt = m_ir_cnt++;
      m_current_scope->insert_variable(name, variable_t(ir_cnt, type));
      pl("%{} = alloca {}", ir_cnt, type_name);

      if (val.has_value()) {
        if (val.type() == typeid(expression_t)) {
          auto [var_ir_cnt, var_type]{std::any_cast<expression_t>(val)};
          if (var_type != type) {
            auto converted_ir_cnt = m_ir_cnt++;
            expression_conversion(var_type, var_ir_cnt, type, converted_ir_cnt);
            var_ir_cnt = converted_ir_cnt;
          }
          pl("store {} %{}, ptr %{}", type_name, var_ir_cnt, ir_cnt);
        } else if (val.type() == typeid(const_expression_t)) {
          auto [var_val, var_type]{std::any_cast<const_expression_t>(val)};
          if (var_type != type)
            var_val = const_expression_conversion(var_type, var_val, type);
          auto var_val_str = to_string(var_val);
          pl("store {} {}, ptr %{}", type_name, var_val_str, ir_cnt);
        } else
          throw; // TODO
      }
    }

    return defaultResult();
  }

  std::any visitNoInitializeVariableDefinition(lgccParser::NoInitializeVariableDefinitionContext *ctx) override {
    std::string name{std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    return std::tuple{name, std::any()};
  }

  std::any
  visitConstExpressionInitializeVariableDefinition(lgccParser::ConstExpressionInitializeVariableDefinitionContext *ctx) override {
    std::string name{std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    auto val = std::any_cast<const_expression_t>(visit(ctx->const_expression()));
    return std::tuple<std::string, std::any>{name, val};
  }

  std::any visitExpressionInitializeVariableDefinition(lgccParser::ExpressionInitializeVariableDefinitionContext *ctx) override {
    std::string name{std::any_cast<std::string>(visit(ctx->IDENTIFIER()))};
    auto val = std::any_cast<expression_t>(visit(ctx->expression()));
    return std::tuple<std::string, std::any>{name, val};
  }

  std::any visitUnaryConstExpression(lgccParser::UnaryConstExpressionContext *ctx) override {
    if (ctx->op->getText() == "+")
      return visit(ctx->const_expression());
    else {
      auto [val, type]{std::any_cast<const_expression_t>(visit(ctx->const_expression()))};
      switch (type) {
      case variable_t::TYPE::INT32:
        return const_expression_t(-std::any_cast<std::int32_t>(val), type);
      case variable_t::TYPE::FLOAT:
        return const_expression_t(-std::any_cast<float>(val), type);
      }
    }
  }

  std::any visitBraceConstExpression(lgccParser::BraceConstExpressionContext *ctx) override {
    return visit(ctx->const_expression());
  }

  std::any visitIntegerConstExpression(lgccParser::IntegerConstExpressionContext *ctx) override {
    std::int32_t val = std::any_cast<std::int32_t>(visit(ctx->LITERAL_INTEGER()));
    return const_expression_t(val, variable_t::TYPE::INT32);
  }

  std::any visitBinaryConstExpression(lgccParser::BinaryConstExpressionContext *ctx) override {
    auto [val1, type1]{std::any_cast<const_expression_t>(visit(ctx->lhs))};
    auto [val2, type2]{std::any_cast<const_expression_t>(visit(ctx->rhs))};
    auto common_type = get_common_type(type1, type2);

    if (type1 != common_type)
      val1 = const_expression_conversion(type1, val1, common_type);
    if (type2 != common_type)
      val2 = const_expression_conversion(type2, val2, common_type);

    switch (common_type) {
    case variable_t::TYPE::INT32:
      switch (ctx->op->getText()[0]) {
      case '+':
        return const_expression_t(std::any_cast<std::int32_t>(val1) + std::any_cast<std::int32_t>(val2), common_type);
      case '-':
        return const_expression_t(std::any_cast<std::int32_t>(val1) - std::any_cast<std::int32_t>(val2), common_type);
      case '*':
        return const_expression_t(std::any_cast<std::int32_t>(val1) * std::any_cast<std::int32_t>(val2), common_type);
      case '/':
        return const_expression_t(std::any_cast<std::int32_t>(val1) / std::any_cast<std::int32_t>(val2), common_type);
      case '%':
        return const_expression_t(std::any_cast<std::int32_t>(val1) % std::any_cast<std::int32_t>(val2), common_type);
      default:
        throw; // TODO
      }
    case variable_t::TYPE::FLOAT:
      switch (ctx->op->getText()[0]) {
      case '+':
        return const_expression_t(std::any_cast<float>(val1) + std::any_cast<float>(val2), common_type);
      case '-':
        return const_expression_t(std::any_cast<float>(val1) - std::any_cast<float>(val2), common_type);
      case '*':
        return const_expression_t(std::any_cast<float>(val1) * std::any_cast<float>(val2), common_type);
      case '/':
        return const_expression_t(std::any_cast<float>(val1) / std::any_cast<float>(val2), common_type);
      default:
        throw; // TODO
      }
    }
  }

  std::any visitUnaryExpression(lgccParser::UnaryExpressionContext *ctx) override {
    if (ctx->op->getText() == "+")
      return visit(ctx->expression());
    else {
      auto [ir_cnt, type] = std::any_cast<expression_t>(visit(ctx->expression()));
      auto cur_ir_cnt = m_ir_cnt++;
      switch (type) {
      case variable_t::TYPE::INT32:
        pl("%{} = mul i32 -1, %{}", cur_ir_cnt, ir_cnt);
        break;
      case variable_t::TYPE::FLOAT:
        pl("%{} = fmul float -1.0, %{}", cur_ir_cnt, ir_cnt);
        break;
      }
      return expression_t(cur_ir_cnt, type);
    }
  }

  std::any visitBinaryExpression(lgccParser::BinaryExpressionContext *ctx) override {
    auto [ir_cnt1, type1] = std::any_cast<expression_t>(visit(ctx->lhs));
    auto [ir_cnt2, type2] = std::any_cast<expression_t>(visit(ctx->rhs));
    auto new_ir_cnt1 = ir_cnt1, new_ir_cnt2 = ir_cnt2;
    auto common_type = get_common_type(type1, type2);

    if (type1 != common_type) {
      new_ir_cnt1 = m_ir_cnt++;
      expression_conversion(type1, ir_cnt1, common_type, new_ir_cnt1);
    }
    if (type2 != common_type) {
      new_ir_cnt2 = m_ir_cnt++;
      expression_conversion(type2, ir_cnt2, common_type, new_ir_cnt2);
    }

    auto cur_ir_cnt = m_ir_cnt++;

    switch (common_type) {
    case variable_t::TYPE::INT32:
      switch (ctx->op->getText()[0]) {
      case '+':
        pl("%{} = add i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '-':
        pl("%{} = sub i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '*':
        pl("%{} = mul i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '/':
        pl("%{} = sdiv i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '%':
        pl("%{} = srem i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      default:
        throw; // TODO: 规范化错误处理
      }
      break;
    case variable_t::TYPE::FLOAT:
      switch (ctx->op->getText()[0]) {
      case '+':
        pl("%{} = fadd i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '-':
        pl("%{} = fsub i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '*':
        pl("%{} = fmul i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      case '/':
        pl("%{} = fdiv i32 %{}, %{}", cur_ir_cnt, new_ir_cnt1, new_ir_cnt2);
        break;
      default:
        throw; // TODO: 规范化错误处理
      }
      break;
    }

    return expression_t(cur_ir_cnt, common_type);
  }

  std::any visitBraceExpression(lgccParser::BraceExpressionContext *ctx) override { return visit(ctx->expression()); }

  std::any visitConstExpressionExpression(lgccParser::ConstExpressionExpressionContext *ctx) override {
    auto [val, type]{std::any_cast<const_expression_t>(visit(ctx->const_expression()))};
    auto ir_cnt = m_ir_cnt++;

    switch (type) {
    case variable_t::TYPE::INT32: {
      auto int32_val = std::any_cast<std::int32_t>(val);
      pl("%{} = add i32 0, {}", ir_cnt, int32_val);
    } break;

    case variable_t::TYPE::FLOAT: {
      auto float_val = std::any_cast<float>(val);
      pl("%{} = fadd float 0.0, {}", ir_cnt, float_val);
    } break;
    }
    return expression_t(ir_cnt, type);
  }

  std::any visitIdentifierExpression(lgccParser::IdentifierExpressionContext *ctx) override {
    auto symbol = m_current_scope->resolve_variable(std::any_cast<std::string>(visit(ctx->IDENTIFIER())));
    auto ir_cnt = symbol.get_ir_cnt();
    auto type = symbol.get_type();
    auto cur_ir_cnt = m_ir_cnt++;

    switch (type) {
    case variable_t::TYPE::INT32:
      pl("%{} = load i32, ptr %{}", cur_ir_cnt, ir_cnt);
      break;

    case variable_t::TYPE::FLOAT:
      pl("%{} = load i32, ptr %{}", cur_ir_cnt, ir_cnt);
      break;
    }
    return expression_t(cur_ir_cnt, type);
  }

  std::any visitTerminal(antlr4::tree::TerminalNode *ctx) override {
    switch (ctx->getSymbol()->getType()) {
    case lgccParser::LITERAL_INTEGER:
      return static_cast<std::int32_t>(std::stoi(ctx->getText(), nullptr, 0));
    case lgccParser::IDENTIFIER:
      return ctx->getText();
    default:
      return ctx->getText();
    }
  }
};

#endif // VISITOR_HPP
