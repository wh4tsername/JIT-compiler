#include <iostream>

#include "parser/parser.h"

int main() {
  JIT_COMPILER::parser::Parser parser;

  std::string expr;
  std::getline(std::cin, expr);
  std::vector<JIT_COMPILER::parser::Token> tokens = parser.Parse(expr);
  for (auto&& token : tokens) {
    if (token.type_ == JIT_COMPILER::parser::Token::FUNCTION) {
      std::cout << "(" << token.function_.name_ << ","
                << token.function_.number_of_arguments_ << ")";
    } else if (token.type_ == JIT_COMPILER::parser::Token::VARIABLE) {
      std::cout << token.variable_.name_;
    } else if (token.type_ == JIT_COMPILER::parser::Token::NUMBER) {
      std::cout << token.number_;
    } else if (token.type_ == JIT_COMPILER::parser::Token::OPERATION) {
      std::cout << static_cast<char>(token.operation_);
    }
    std::cout << " ";
  }

  return 0;
}
