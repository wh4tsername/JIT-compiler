#include "arm_compiler.h"

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
  while (externs->name_ != nullptr && externs->pointer_ != nullptr) {
    externs_[externs->name_] = externs->pointer_;
    ++externs;
  }

  auto instructions = GetInstructions(tokens);

  auto it = static_cast<uint32_t*>(out_buffer);
  for (auto&& instruction : instructions) {
    *it = instruction;
    ++it;
  }
}

void JIT_COMPILER::jit_compiler::JitCompiler::MoveConstant(uint32_t constant) {
  instructions_.emplace_back(MOVW_R0 |
                             ((((constant & ((1 << 16) - 1)) >> 12) << 16) |
                              (constant & ((1 << 16) - 1) & ((1 << 12) - 1))));
  instructions_.emplace_back(MOVT_R0 | (((constant >> 28) << 16) |
                                        ((constant >> 16) & ((1 << 12) - 1))));
}

void JIT_COMPILER::jit_compiler::JitCompiler::MoveAddress(uint32_t constant) {
  instructions_.emplace_back(MOVW_R4 |
                             ((((constant & ((1 << 16) - 1)) >> 12) << 16) |
                              (constant & ((1 << 16) - 1) & ((1 << 12) - 1))));
  instructions_.emplace_back(MOVT_R4 | (((constant >> 28) << 16) |
                                        ((constant >> 16) & ((1 << 12) - 1))));
}

void JIT_COMPILER::jit_compiler::JitCompiler::LoadVariable(
    void* pointer_of_variable) {
  MoveConstant(reinterpret_cast<uint32_t>(pointer_of_variable));
  instructions_.emplace_back(LDR_R0_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CallFunction(
    void* function_pointer, size_t number_of_arguments) {
  for (size_t i = number_of_arguments; i > 0; --i) {
    instructions_.emplace_back(POP_TO[i - 1]);
  }

  MoveAddress(reinterpret_cast<uint32_t>(function_pointer));
  instructions_.emplace_back(BLX_R4);

  instructions_.emplace_back(PUSH_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::ExecuteOperation(
    const parser::Operation& operation) {
  if (operation == JIT_COMPILER::parser::Operation::UNARY_MINUS) {
    instructions_.emplace_back(POP_TO[1]);
    instructions_.emplace_back(NEG_R0_R1);
    instructions_.emplace_back(PUSH_R0);
    return;
  }
  instructions_.emplace_back(POP_TO[1]);

  instructions_.emplace_back(POP_TO[0]);

  if (operation == parser::Operation::ADD) {
    instructions_.emplace_back(ADD_R0_R1);
  } else if (operation == parser::Operation::SUBSTRACT) {
    instructions_.emplace_back(SUB_R0_R1);
  } else if (operation == parser::Operation::MULTIPLY) {
    instructions_.emplace_back(MUL_R0_R1);
  }

  instructions_.emplace_back(PUSH_R0);
}

std::vector<uint32_t> JIT_COMPILER::jit_compiler::JitCompiler::GetInstructions(
    const std::vector<JIT_COMPILER::parser::Token>& tokens) {
  instructions_.emplace_back(PUSH_R4_LR);

  for (auto&& token : tokens) {
    if (token.type_ == JIT_COMPILER::parser::Token::NUMBER) {
      MoveConstant(token.number_);
      instructions_.emplace_back(PUSH_R0);
    } else if (token.type_ == JIT_COMPILER::parser::Token::VARIABLE) {
      LoadVariable(externs_[token.name_]);
      instructions_.emplace_back(PUSH_R0);
    } else if (token.type_ == JIT_COMPILER::parser::Token::FUNCTION) {
      CallFunction(externs_[token.name_], token.number_of_arguments_);
    } else {
      ExecuteOperation(token.operation_);
    }
  }

  instructions_.emplace_back(POP_TO[0]);
  instructions_.emplace_back(POP_R4_PC);

  return instructions_;
}
