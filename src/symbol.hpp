#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <map>
#include <memory>
#include <string>

class symbol {
private:
  std::string m_name;

public:
  symbol(const std::string &name) : m_name(name) {}

  std::string get_name() { return m_name; }
};

class scope {
private:
  std::map<std::string, std::unique_ptr<symbol>> symbol_table;
  scope *m_father{nullptr};

public:
  scope(scope *father = nullptr) : m_father(father) {}

  void insert(const std::string &name, const symbol &sym) {
    if (symbol_table.contains(name))
      throw false; // TODO: 规范抛出异常
    symbol_table.emplace(name, new symbol(sym));
  }

  const symbol &resolve(const std::string &name) {
    if (auto it = symbol_table.find(name); it != symbol_table.end())
      return *(it->second);
    if (m_father)
      return m_father->resolve(name);
    throw false; // TODO: 规范抛出异常
  }
};

#endif // SYMBOL_HPP
