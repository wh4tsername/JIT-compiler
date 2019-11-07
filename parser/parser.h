#pragma once

#include <stack>
#include <string>
#include <vector>
#include <iostream>

namespace JIT_COMPILER::parser {
enum struct Operation {
  ADD = '+',
  SUBSTRACT = '-',
  MULTIPLY = '*',
  OPEN_BRACKET = '(',
  CLOSE_BRACKET = ')',
  COMMA = ',',
  UNARY_MINUS = '~'
};

struct Token {
  enum Type { VARIABLE, FUNCTION, NUMBER, OPERATION } type_;

  Operation operation_;
  std::string name_;
  size_t number_of_arguments_;
  int32_t number_;
};

class ParserTest;

class Parser {
 public:
  std::vector<Token> Parse(const std::string& expression);

  friend ParserTest;

 private:
  static size_t GetOperationPriority(const Operation& operation);

  void Split();

  void GetPostfixNotation();

  std::string expression_;
  std::vector<Token> tokens_;
  std::vector<Token> postfix_notation_;
};

class ParserTest {
 public:
  static void Test(Parser& parser, const std::string& test);
};
}  // namespace JIT_COMPILER::parser
