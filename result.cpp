#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace JIT_COMPILER::parser
{
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

class Parser
{
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
} // namespace JIT_COMPILER::parser

std::vector<JIT_COMPILER::parser::Token>
JIT_COMPILER::parser::Parser::Parse(const std::string& expression)
{
    expression_ = expression;
    Split();
    GetPostfixNotation();

    return postfix_notation_;
}

size_t JIT_COMPILER::parser::Parser::GetOperationPriority(
    const parser::Operation& operation)
{
    if (operation == Operation::OPEN_BRACKET) {
        return 0;
    }
    if (operation == Operation::CLOSE_BRACKET ||
        operation == Operation::COMMA) {
        return 1;
    }
    if (operation == Operation::ADD || operation == Operation::SUBSTRACT) {
        return 2;
    }
    if (operation == Operation::MULTIPLY) {
        return 3;
    }
    if (operation == Operation::UNARY_MINUS) {
        return 4;
    }
    return 5;
}

void JIT_COMPILER::parser::Parser::Split()
{
    for (size_t i = 0; i < expression_.size();) {
        if (isspace(expression_[i])) {
            ++i;
            continue;
        }

        if (isalpha(expression_[i])) {
            std::string name;
            while (i < expression_.size() &&
                   (isalpha(expression_[i]) || isdigit(expression_[i]))) {
                name.push_back(expression_[i]);
                ++i;
            }

            if (i < expression_.size() &&
                expression_[i] == static_cast<char>(Operation::OPEN_BRACKET)) {
                size_t j = i + 1;
                size_t bracket_balance = 1;
                size_t number_of_arguments = 0;
                while (bracket_balance > 0) {
                    if (expression_[j] ==
                        static_cast<char>(Operation::OPEN_BRACKET)) {
                        ++bracket_balance;
                    } else if (
                        expression_[j] ==
                        static_cast<char>(Operation::CLOSE_BRACKET)) {
                        --bracket_balance;
                    } else if (
                        expression_[j] == static_cast<char>(Operation::COMMA) &&
                        bracket_balance == 1) {
                        ++number_of_arguments;
                    }
                    ++j;
                }
                ++number_of_arguments;

                Token token;
                token.type_ = Token::FUNCTION;
                token.name_ = name;
                token.number_of_arguments_ = number_of_arguments;
                tokens_.emplace_back(token);
            } else {
                Token token;
                token.type_ = Token::VARIABLE;
                token.name_ = name;
                tokens_.emplace_back(token);
            }
        } else if (isdigit(expression_[i])) {
            std::string number_string;
            while (i < expression_.size() && isdigit(expression_[i])) {
                number_string.push_back(expression_[i]);
                ++i;
            }

            Token token;
            token.type_ = Token::NUMBER;
            token.number_ = std::strtol(number_string.c_str(), nullptr, 10);
            tokens_.emplace_back(token);
        } else {
            Token token;
            token.type_ = Token::OPERATION;
            token.operation_ = static_cast<Operation>(expression_[i]);
            tokens_.emplace_back(token);
            ++i;
        }
    }
}

void JIT_COMPILER::parser::Parser::GetPostfixNotation()
{
    std::stack<Token> operators;

    bool is_next_token_operand = true;
    for (auto&& token : tokens_) {
        if (is_next_token_operand) {
            if (token.type_ == Token::OPERATION) {
                if (token.operation_ == Operation::SUBSTRACT) {
                    token.operation_ = Operation::UNARY_MINUS;
                }
                operators.push(token);
            } else if (token.type_ == Token::FUNCTION) {
                operators.push(token);
            } else {
                postfix_notation_.emplace_back(token);
                is_next_token_operand = false;
            }
        } else {
            while (!operators.empty()) {
                if (GetOperationPriority(operators.top().operation_) <
                    GetOperationPriority(token.operation_)) {
                    break;
                }
                postfix_notation_.emplace_back(operators.top());
                operators.pop();
            }
            if (token.operation_ == Operation::CLOSE_BRACKET) {
                operators.pop();
                if (!operators.empty() &&
                    operators.top().type_ == Token::FUNCTION) {
                    postfix_notation_.emplace_back(operators.top());
                    operators.pop();
                }
                continue;
            }
            if (token.operation_ != Operation::COMMA) {
                operators.push(token);
            }
            is_next_token_operand = true;
        }
    }
    while (!operators.empty()) {
        if (GetOperationPriority(operators.top().operation_) <
            GetOperationPriority(Operation::CLOSE_BRACKET)) {
            break;
        }
        postfix_notation_.emplace_back(operators.top());
        operators.pop();
    }
}

typedef struct {
    const char* name;
    void* pointer;
} symbol_t;

extern "C" void jit_compile_expression_to_arm(
    const char* expression,
    const symbol_t* externs,
    void* out_buffer);

namespace JIT_COMPILER::jit_compiler
{
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

class JitCompiler
{
  public:
    void Compile(
        const std::vector<JIT_COMPILER::parser::Token>& tokens,
        const symbol_t* externs,
        void* out_buffer);

  private:
    std::vector<uint32_t>
    GetInstructions(const std::vector<JIT_COMPILER::parser::Token>& tokens);

    void MoveConstant(uint32_t constant);
    void MoveAddress(uint32_t constant);
    void LoadVariable(void* pointer_of_variable);
    void CallFunction(void* function_pointer, size_t number_of_arguments);
    void ExecuteOperation(const parser::Operation& operation);

    std::unordered_map<std::string, void*> externs_;
    std::vector<uint32_t> instructions_;
};
} // namespace JIT_COMPILER::jit_compiler

extern "C" void jit_compile_expression_to_arm(
    const char* expression,
    const symbol_t* externs,
    void* out_buffer)
{
    JIT_COMPILER::parser::Parser parser;
    std::vector<JIT_COMPILER::parser::Token> tokens_in_postfix_notation =
        parser.Parse(expression);

    JIT_COMPILER::jit_compiler::JitCompiler compiler;
    compiler.Compile(tokens_in_postfix_notation, externs, out_buffer);
}

void JIT_COMPILER::jit_compiler::JitCompiler::Compile(
    const std::vector<JIT_COMPILER::parser::Token>& tokens,
    const symbol_t* externs,
    void* out_buffer)
{
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

void JIT_COMPILER::jit_compiler::JitCompiler::MoveConstant(uint32_t constant)
{
    instructions_.emplace_back(
        MOVW_R0 | ((((constant & ((1 << 16) - 1)) >> 12) << 16) |
                   (constant & ((1 << 16) - 1) & ((1 << 12) - 1))));
    instructions_.emplace_back(
        MOVT_R0 |
        (((constant >> 28) << 16) | ((constant >> 16) & ((1 << 12) - 1))));
}

void JIT_COMPILER::jit_compiler::JitCompiler::MoveAddress(uint32_t constant)
{
    instructions_.emplace_back(
        MOVW_R4 | ((((constant & ((1 << 16) - 1)) >> 12) << 16) |
                   (constant & ((1 << 16) - 1) & ((1 << 12) - 1))));
    instructions_.emplace_back(
        MOVT_R4 |
        (((constant >> 28) << 16) | ((constant >> 16) & ((1 << 12) - 1))));
}

void JIT_COMPILER::jit_compiler::JitCompiler::LoadVariable(
    void* pointer_of_variable)
{
    MoveConstant(reinterpret_cast<uint32_t>(pointer_of_variable));
    instructions_.emplace_back(LDR_R0_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::CallFunction(
    void* function_pointer,
    size_t number_of_arguments)
{
    for (size_t i = number_of_arguments; i > 0; --i) {
        instructions_.emplace_back(POP_TO[i - 1]);
    }

    MoveAddress(reinterpret_cast<uint32_t>(function_pointer));
    instructions_.emplace_back(BLX_R4);

    instructions_.emplace_back(PUSH_R0);
}

void JIT_COMPILER::jit_compiler::JitCompiler::ExecuteOperation(
    const parser::Operation& operation)
{
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
    const std::vector<JIT_COMPILER::parser::Token>& tokens)
{
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
