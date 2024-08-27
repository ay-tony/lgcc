#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <concepts>
#include <map>
#include <memory>
#include <string>

class variable_t {
private:
  uint32_t m_ir_cnt;

public:
  variable_t(uint32_t ir_cnt) : m_ir_cnt(ir_cnt) {}

  uint32_t get_ir_cnt() const { return m_ir_cnt; }
};

class scope_t {
private:
  std::map<std::string, std::unique_ptr<variable_t>> variable_table;
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
    throw false; // TODO: 规范抛出异常
  }
};

#endif // SYMBOL_HPP
