#include <iostream>

#include "../parser/parser.h"\

int main() {
  JIT_COMPILER::parser::Parser parser;

  std::string expr;
  std::getline(std::cin, expr);

  JIT_COMPILER::parser::ParserTest parser_test;
  parser_test.Test(parser, expr);

  return 0;
}
