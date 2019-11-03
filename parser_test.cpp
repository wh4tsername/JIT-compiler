#include <iostream>

#include "parser/parser.h"

int main() {
  parser::Parser parser;

  std::string expr;
  std::getline(std::cin, expr);
  std::vector<parser::Token> tokens = parser(expr);
  for (auto&& tok : tokens) {
    if (tok.type_ == parser::Token::FUNCTION) {
      std::cout << "(" << tok.function_.name_ << ","
                << tok.function_.number_of_arguments_ << ")";
    } else if (tok.type_ == parser::Token::VARIABLE) {
      std::cout << tok.variable_.name_;
    } else if (tok.type_ == parser::Token::NUMBER) {
      std::cout << tok.number_;
    } else if (tok.type_ == parser::Token::OPERATION) {
      std::cout << static_cast<char>(tok.operation_);
    }
    std::cout << " ";
  }

  return 0;
}
