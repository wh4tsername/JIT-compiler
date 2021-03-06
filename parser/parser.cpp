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
  if (operation == Operation::OPEN_BRACKET) {
    return 0;
  }
  if (operation == Operation::CLOSE_BRACKET || operation == Operation::COMMA) {
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

void JIT_COMPILER::parser::Parser::Split() {
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

void JIT_COMPILER::parser::Parser::GetPostfixNotation() {
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
        if (!operators.empty() && operators.top().type_ == Token::FUNCTION) {
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

void JIT_COMPILER::parser::ParserTest::Test(JIT_COMPILER::parser::Parser &parser,
                                            const std::string &test) {
  parser.expression_ = test;

  parser.Split();

  for (auto&& token : parser.tokens_) {
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
}
