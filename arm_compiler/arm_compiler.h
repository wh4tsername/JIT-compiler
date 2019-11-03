#pragma once

#include <unordered_map>
#include "../parser/parser.h"

typedef struct {
  const char* name;
  void* pointer;
} symbol_t;

extern "C" void jit_compile_expression_to_arm(const char* expression,
                                              const symbol_t* externs,
                                              void* out_buffer);

namespace JIT_COMPILER::jit_compiler {
uint32_t ADD_R0_R1 = 0xE0800001;
uint32_t SUB_R0_R1 = 0xE0400001;
uint32_t MUL_R0_R1 = 0xE0000190;
uint32_t NEG_R0_R1 = 0xE2610000;
uint32_t PUSH_R0 = 0xE52D0004;
uint32_t POP[4] = {0xE49D0004, 0xE49D1004, 0xE49D2004, 0xE49D3004};
uint32_t MOVW[5] = {0xE3000000, 0xE3001000, 0xE3002000, 0xE3003000, 0xE3004000};
uint32_t MOVT[5] = {0xE3400000, 0xE3401000, 0xE3402000, 0xE3403000, 0xE3404000};
uint32_t LDR[5] = {0xE5900000, 0xE5911000, 0xE5922000, 0xE5933000, 0xE5944000};
uint32_t BLX_R4 = 0xE12FFF34;
uint32_t PUSH_R4_LR = 0xE92D4010;
uint32_t POP_R4_PC = 0xE8bd8010;

class JitCompiler {
 public:
  void Compile(const std::vector<JIT_COMPILER::parser::Token>& tokens,
               const symbol_t* externs, void* out_buffer);

 private:
  std::vector<uint32_t> GetInstructions(
      const std::vector<JIT_COMPILER::parser::Token>& tokens);

  void MoveConstant(size_t reg, uint32_t constant);
  void LoadVariable(size_t reg, void* pointer_of_variable);
  void CallFunction(void* function_pointer);
  void CallFunction(void* function_pointer, size_t number_of_arguments);
  void CompleteBinaryOperation(const parser::Operation& operation);
  void CompleteUnaryMinus();

  std::unordered_map<std::string, void*> externs_;
  std::vector<uint32_t> instructions_;
};
}  // namespace JIT_COMPILER::jit_compiler
