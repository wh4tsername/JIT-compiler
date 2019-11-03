#include "arm_compiler.h"

#include <iostream>

extern "C" void jit_compile_expression_to_arm(const char* expression,
                                              const symbol_t* externs,
                                              void* out_buffer) {
  JIT_COMPILER::parser::Parser parser;
  std::vector<JIT_COMPILER::parser::Token> tokens_in_postfix_notation =
      parser.Parse(expression);

  JIT_COMPILER::jit_compiler::JitCompiler compiler;
  compiler.Compile(tokens_in_postfix_notation, externs, out_buffer);
}

void JIT_COMPILER::jit_compiler::JitCompiler::Compile(
    const std::vector<JIT_COMPILER::parser::Token>& tokens,
    const symbol_t* externs, void* out_buffer) {
  while (externs->name != nullptr && externs->pointer != nullptr) {
    externs_[externs->name] = externs->pointer;
    ++externs;
  }

  auto instructions = GetInstructions(tokens);

  auto it = static_cast<uint32_t*>(out_buffer);
  for (auto&& instruction : instructions) {
    *it = instruction;
    ++it;
  }
}

void JIT_COMPILER::jit_compiler::JitCompiler::MoveConstant(size_t reg,
                                                           uint32_t constant) {
  instructions_.emplace_back(MOVW[reg] |
                             ((((constant & ((1 << 16) - 1)) >> 12) << 16) |
                              (constant & ((1 << 16) - 1) & ((1 << 12) - 1))));
  instructions_.emplace_back(
      MOVT[reg] |
      (((constant >> 28) << 16) | ((constant >> 16) & ((1 << 12) - 1))));
}

void JIT_COMPILER::jit_compiler::JitCompiler::LoadVariable(
    size_t reg, void* pointer_of_variable) {
  MoveConstant(reg, reinterpret_cast<uint32_t>(pointer_of_variable));
  instructions_.emplace_back(LDR[reg]);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CallFunction(
    void* function_pointer) {
  MoveConstant(4, reinterpret_cast<uint32_t>(function_pointer));
  instructions_.emplace_back(BLX_R4);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CallFunction(
    void* function_pointer, size_t number_of_arguments) {
  for (size_t i = number_of_arguments; i > 0; --i) {
    instructions_.emplace_back(POP[i - 1]);
  }
  CallFunction(function_pointer);

  instructions_.emplace_back(PUSH_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CompleteBinaryOperation(
    const parser::Operation& operation) {
  instructions_.emplace_back(POP[1]);
  instructions_.emplace_back(POP[0]);

  switch (operation) {
    case parser::Operation::PLUS:
      instructions_.emplace_back(ADD_R0_R1);
      break;

    case parser::Operation::MINUS:
      instructions_.emplace_back(SUB_R0_R1);
      break;

    case parser::Operation::MULTIPLY:
      instructions_.emplace_back(MUL_R0_R1);
      break;
  }

  instructions_.emplace_back(PUSH_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CompleteUnaryMinus() {
  instructions_.emplace_back(POP[1]);
  instructions_.emplace_back(NEG_R0_R1);
  instructions_.emplace_back(PUSH_R0);
}

std::vector<uint32_t> JIT_COMPILER::jit_compiler::JitCompiler::GetInstructions(
    const std::vector<JIT_COMPILER::parser::Token>& tokens) {
  instructions_.emplace_back(PUSH_R4_LR);

  for (auto&& token : tokens) {
    if (token.type_ == JIT_COMPILER::parser::Token::NUMBER) {
      MoveConstant(0, token.number_);
      instructions_.emplace_back(PUSH_R0);
    } else if (token.type_ == JIT_COMPILER::parser::Token::VARIABLE) {
      LoadVariable(0, externs_[token.variable_.name_]);
      instructions_.emplace_back(PUSH_R0);
    } else if (token.type_ == JIT_COMPILER::parser::Token::FUNCTION) {
      CallFunction(externs_[token.function_.name_],
                   token.function_.number_of_arguments_);
    } else if (token.operation_ ==
               JIT_COMPILER::parser::Operation::UNARY_MINUS) {
      CompleteUnaryMinus();
    } else {
      CompleteBinaryOperation(token.operation_);
    }
  }

  instructions_.emplace_back(POP[0]);
  instructions_.emplace_back(POP_R4_PC);

  return instructions_;
}
