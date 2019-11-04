#pragma once

#include <unordered_map>
#include "../parser/parser.h"

typedef struct {
  const char* name_;
  void* pointer_;
} symbol_t;

extern "C" void jit_compile_expression_to_arm(const char* expression,
                                              const symbol_t* externs,
                                              void* out_buffer);

namespace JIT_COMPILER::jit_compiler {
const uint32_t ADD_R0_R1 = 0xE0800001;
const uint32_t SUB_R0_R1 = 0xE0400001;
const uint32_t MUL_R0_R1 = 0xE0000190;
const uint32_t NEG_R0_R1 = 0xE2610000;
const uint32_t PUSH_R0 = 0xE52D0004;
const uint32_t MOVW_R0 = 0xE3000000;
const uint32_t MOVW_R4 = 0xE3004000;
const uint32_t MOVT_R0 = 0xE3400000;
const uint32_t MOVT_R4 = 0xE3404000;
const uint32_t POP_TO[4] = {0xE49D0004, 0xE49D1004, 0xE49D2004, 0xE49D3004};
const uint32_t LDR_R0_R0 = 0xE5900000;
const uint32_t BLX_R4 = 0xE12FFF34;
const uint32_t PUSH_R4_LR = 0xE92D4010;
const uint32_t POP_R4_PC = 0xE8bd8010;

class JitCompiler {
 public:
  void Compile(const std::vector<JIT_COMPILER::parser::Token>& tokens,
               const symbol_t* externs, void* out_buffer);

 private:
  std::vector<uint32_t> GetInstructions(
      const std::vector<JIT_COMPILER::parser::Token>& tokens);

  void MoveConstant(uint32_t constant);
  void MoveAddress(uint32_t constant);
  void LoadVariable(void* pointer_of_variable);
  void CallFunction(void* function_pointer, size_t number_of_arguments);
  void ExecuteOperation(const parser::Operation& operation);

  std::unordered_map<std::string, void*> externs_;
  std::vector<uint32_t> instructions_;
};
}  // namespace JIT_COMPILER::jit_compiler
