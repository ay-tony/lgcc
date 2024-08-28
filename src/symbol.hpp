#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <any>
#include <concepts>
#include <map>
#include <memory>
#include <string>

// TODO: 检查所有用 string 的地方能不能用 string_view 代替

class variable_t {
public:
  using ir_cnt_t = std::uint32_t; // TODO: 本语句放在哪里有待商榷
  enum class TYPE { INT32, FLOAT };

  static TYPE to_type(std::string &str) {
    if (str == "int")
      return TYPE::INT32;
    else if (str == "float")
      return TYPE::FLOAT;
    else
      throw; // TODO: 规范处理异常
  }

  static std::string to_string(TYPE type) {
    using namespace std::literals::string_literals;
    switch (type) {
    case TYPE::INT32:
      return "i32"s;
    case TYPE::FLOAT:
      return "float"s;
    }
  }

private:
  ir_cnt_t m_ir_cnt;
  TYPE m_type;

public:
  variable_t(std::uint32_t ir_cnt, TYPE type) : m_ir_cnt(ir_cnt), m_type(type) {}

  ir_cnt_t get_ir_cnt() const { return m_ir_cnt; }
  TYPE get_type() const { return m_type; }
};

class function_t {
public:
  enum class TYPE { VOID, INT32, FLOAT };

  static TYPE to_type(std::string &str) {
    if (str == "void")
      return TYPE::VOID;
    else if (str == "int")
      return TYPE::INT32;
    else if (str == "float")
      return TYPE::FLOAT;
    else
      throw; // TODO: 规范处理异常
  }

  static std::string to_string(TYPE type) {
    using namespace std::literals::string_literals;
    switch (type) {
    case TYPE::VOID:
      return "void"s;
    case TYPE::INT32:
      return "i32"s;
    case TYPE::FLOAT:
      return "float"s;
    }
  }

  static variable_t::TYPE to_variable_type(TYPE return_type) {
    switch (return_type) {
    case TYPE::INT32:
      return variable_t::TYPE::INT32;
    case TYPE::FLOAT:
      return variable_t::TYPE::FLOAT;
    case TYPE::VOID:
      throw; // TODO
    }
  }

private:
  TYPE m_return_type;

public:
  function_t(TYPE return_type) : m_return_type(return_type) {}

  TYPE get_return_type() const { return m_return_type; }
};

class scope_t {
private:
  std::map<std::string, std::unique_ptr<variable_t>> variable_table;
  std::map<std::string, std::unique_ptr<function_t>> function_table;
  scope_t *m_father{nullptr};

public:
  scope_t(scope_t *father = nullptr) : m_father(father) {}

  template <class T>
    requires std::derived_from<T, variable_t>
  void insert_variable(const std::string &name, const T &sym) {
    if (variable_table.contains(name))
      throw false; // TODO: 规范抛出异常
    variable_table.emplace(name, new T(sym));
  }

  const variable_t &resolve_variable(const std::string &name) {
    if (auto it = variable_table.find(name); it != variable_table.end())
      return *(it->second);
    if (m_father)
      return m_father->resolve_variable(name);
    throw; // TODO: 规范抛出异常
  }

  template <class T>
    requires std::derived_from<T, function_t>
  void insert_function(const std::string &name, const T &sym) {
    if (function_table.contains(name))
      throw; // TODO: 规范抛出异常
    function_table.emplace(name, new T(sym));
  }

  const function_t &resolve_function(const std::string &name) {
    if (auto it = function_table.find(name); it != function_table.end())
      return *(it->second);
    if (m_father)
      return m_father->resolve_function(name);
    throw; // TODO: 规范抛出异常
  }
};

#endif // SYMBOL_HPP
