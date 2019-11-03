#pragma once

#include <stack>
#include <string>
#include <vector>

namespace JIT_COMPILER::parser {
enum struct Operation {
  PLUS = '+',
  MINUS = '-',
  MULTIPLY = '*',
  UNARY_MINUS = '~',
  OPEN_BRACKET = '(',
  CLOSE_BRACKET = ')',
  COMMA = ','
};

struct Variable {
  std::string name_;
};

struct Function {
  std::string name_;
  size_t number_of_arguments_;
};

struct Token {
  enum Type { VARIABLE, FUNCTION, NUMBER, OPERATION } type_;

  Operation operation_;
  Variable variable_;
  Function function_;
  int32_t number_;
};

class Parser {
 public:
  std::vector<Token> Parse(const std::string& expression);

 private:
  size_t GetOperationPriority(const Operation& operation);

  void Split();

  void GetPostfixNotation();

  std::string expression_;
  std::vector<Token> tokens_;
  std::vector<Token> postfix_notation_;
};
}  // namespace JIT_COMPILER::parser
