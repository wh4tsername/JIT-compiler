#include "parser.h"

std::vector<JIT_COMPILER::parser::Token> JIT_COMPILER::parser::Parser::Parse(
    const std::string& expression) {
  expression_ = expression;
  Split();
  GetPostfixNotation();

  return postfix_notation_;
}

size_t JIT_COMPILER::parser::Parser::GetOperationPriority(
    const parser::Operation& operation) {
  switch (operation) {
    case Operation::OPEN_BRACKET:
      return 0;

    case Operation::CLOSE_BRACKET:
    case Operation::COMMA:
      return 1;

    case Operation::PLUS:
    case Operation::MINUS:
      return 2;

    case Operation::MULTIPLY:
      return 3;

    case Operation::UNARY_MINUS:
      return 4;

    default:
      return 5;
  }
}

void JIT_COMPILER::parser::Parser::Split() {
  for (size_t i = 0; i < expression_.size();) {
    if (isspace(expression_[i])) {
      ++i;
      continue;
    }

    if (isdigit(expression_[i])) {
      std::string number_string;
      while (i < expression_.size() && isdigit(expression_[i])) {
        number_string.push_back(expression_[i]);
        ++i;
      }

      Token token;
      token.type_ = Token::NUMBER;
      token.number_ = std::strtol(number_string.c_str(), nullptr, 10);
      tokens_.emplace_back(token);
    } else if (isalpha(expression_[i])) {
      std::string name;
      while (i < expression_.size() &&
             (isalpha(expression_[i]) || isdigit(expression_[i]))) {
        name.push_back(expression_[i]);
        ++i;
      }

      if (i >= expression_.size() ||
          expression_[i] != static_cast<char>(Operation::OPEN_BRACKET)) {
        Token token;
        token.type_ = Token::VARIABLE;
        token.variable_.name_ = name;
        tokens_.emplace_back(token);
      } else {
        size_t j = i + 1;
        size_t bracket_balance = 1;
        size_t number_of_arguments = 0;
        while (bracket_balance > 0) {
          if (expression_[j] == static_cast<char>(Operation::OPEN_BRACKET)) {
            ++bracket_balance;
          } else if (expression_[j] ==
                     static_cast<char>(Operation::CLOSE_BRACKET)) {
            --bracket_balance;
          } else if (expression_[j] == static_cast<char>(Operation::COMMA) &&
                     bracket_balance == 1) {
            ++number_of_arguments;
          }
          ++j;
        }
        ++number_of_arguments;

        Token token;
        token.type_ = Token::FUNCTION;
        token.function_.name_ = name;
        token.function_.number_of_arguments_ = number_of_arguments;
        tokens_.emplace_back(token);
      }
    } else {
      Token token;
      token.type_ = Token::OPERATION;
      token.operation_ = static_cast<Operation>(expression_[i]);
      tokens_.emplace_back(token);
      ++i;
    }
  }
}

void JIT_COMPILER::parser::Parser::GetPostfixNotation() {
  std::stack<Token> operators;

  bool is_next_token_operand = true;
  for (auto&& token : tokens_) {
    if (is_next_token_operand) {
      if (token.type_ == Token::OPERATION) {
        if (token.operation_ == Operation::MINUS) {
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
        postfix_notation_.push_back(operators.top());
        operators.pop();
      }
      if (token.operation_ == Operation::CLOSE_BRACKET) {
        operators.pop();
        if (!operators.empty() && operators.top().type_ == Token::FUNCTION) {
          postfix_notation_.push_back(operators.top());
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
    postfix_notation_.push_back(operators.top());
    operators.pop();
  }
}
