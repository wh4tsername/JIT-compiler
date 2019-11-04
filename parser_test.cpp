#include <iostream>

#include "parser/parser.h"
#include "arm_compiler/arm_compiler.h"

int main() {
  JIT_COMPILER::parser::Parser parser;

  std::string expr;
  std::getline(std::cin, expr);
  std::vector<JIT_COMPILER::parser::Token> tokens = parser.Parse(expr);
  for (auto&& token : tokens) {
    if (token.type_ == JIT_COMPILER::parser::Token::FUNCTION) {
      std::cout << "(" << token.name_ << ","
                << token.number_of_arguments_ << ")";
    } else if (token.type_ == JIT_COMPILER::parser::Token::VARIABLE) {
      std::cout << token.name_;
    } else if (token.type_ == JIT_COMPILER::parser::Token::NUMBER) {
      std::cout << token.number_;
    } else if (token.type_ == JIT_COMPILER::parser::Token::OPERATION) {
      std::cout << static_cast<char>(token.operation_);
    }
    std::cout << " ";
  }
  std::cout << std::endl;

  JIT_COMPILER::jit_compiler::JitCompiler compiler;

  auto externs = new symbol_t[1];
  externs[0].name_ = nullptr;
  externs[0].pointer_ = nullptr;

  void* buffer = new char[1000];

  compiler.Compile(tokens, externs, buffer);

  std::cout << "ok";

  return 0;
}
