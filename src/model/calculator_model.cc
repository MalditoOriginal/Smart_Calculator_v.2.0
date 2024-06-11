#include "calculator_model.h"

#include <cmath>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

namespace s21 {
void CalculatorModel::Calculate() {
  if (InfixToPostfix() != err) {
    for (auto it = postfix_.begin(); it != postfix_.end(); ++it) {
      if (*it == ' ')
        continue;
      else if (isNumber(it))
        NumberToStack(it);
      else if (isOperators(it))
        CalcSimple(it);
      else
        CalcFunc(it);
      if (status_ == ok && !stack_.empty()) result_ = stack_.top();
    }
  }

  if (status_ == ok && !stack_.empty()) stack_.pop();
  if (!stack_.empty()) status_ = err;
}

enum Status CalculatorModel::InfixToPostfix() {
  if (!isValid()) return err;
  zeroOut();

  for (auto it = infix_.begin(); it != infix_.end(); ++it) {
    if (*it == ' ')
      continue;
    else if (isNumber(it))
      NumberToPostfix(it);
    else if (isUnary(it))
      UnaryToPostFix(it);
    else if (isBrackets(it))
      BracketsToPostfix(it);
    else if (isFunction(it))
      FunctionToPostfix(it);
    else if (isPow(it))
      PowToPostfix(it);
    else if (isOperators(it)) {
      while (!stack_.empty() && Priority(*it) <= Priority(stack_.top()))
        PushBackPostfix();
      stack_.push(*it);
    } else
      status_ = err;
  }

  while (!stack_.empty()) PushBackPostfix();
  if (status_ != ok) postfix_ = "";

  return status_;
}

void CalculatorModel::NumberToStack(std::string::iterator &it) {
  if (*it == 'x')
    stack_.push(x_);
  else {
    std::string digit;
    while (isNumber(it)) digit.push_back(*it++);
    *it--;
    try {
      stack_.push(std::stod(digit));
    } catch (const std::out_of_range &e) {
      status_ = err;
    }
  }
}

void CalculatorModel::CalcSimple(std::string::iterator &it) {
  long double Op2 = 0, Op1 = 0;

  // Check if the stack has at least two elements
  if (stack_.size() < 2) {
    status_ = err;
    return;
  }

  // Get the top two elements from the stack
  Op2 = stack_.top();
  stack_.pop();
  Op1 = stack_.top();
  stack_.pop();

  // Perform the operation based on the iterator value
  switch (*it) {
    case '+':
      stack_.push(Op1 + Op2);
      break;
    case '-':
      stack_.push(Op1 - Op2);
      break;
    case '*':
      stack_.push(Op1 * Op2);
      break;
    case '/':
      if (Op2 != 0)
        stack_.push(Op1 / Op2);
      else
        status_ = err;
      break;
    case '^':
      stack_.push(std::pow(Op1, Op2));
      break;
    case '%':
      stack_.push(std::fmod(Op1, Op2));
      break;
    default:
      status_ = err;
      break;
  }
}

void CalculatorModel::CalcFunc(std::string::iterator &it) {
  // Extract the operand from the stack
  long double operand = 0;
  if (stack_.empty())
    status_ = err;
  else {
    operand = stack_.top();
    stack_.pop();
  }

  // Apply the corresponding function based on the iterator value
  switch (*it) {
    case 'A':
      stack_.push(std::cos(operand));
      break;
    case 'B':
      stack_.push(std::sin(operand));
      break;
    case 'C':
      stack_.push(std::tan(operand));
      break;
    case 'D':
      stack_.push(std::acos(operand));
      break;
    case 'E':
      stack_.push(std::asin(operand));
      break;
    case 'F':
      stack_.push(std::atan(operand));
      break;
    case 'G':
      // Check if operand is non-negative before taking square root
      if (operand >= 0) {
        stack_.push(std::sqrt(operand));
      } else {
        status_ = err;
      }
      break;
    case 'H':
      // Check if operand is positive before taking base-10 logarithm
      if (operand > 0)
        stack_.push(std::log10(operand));
      else
        status_ = err;
      break;
    case 'I':
      // Check if operand is positive before taking natural logarithm
      if (operand > 0)
        stack_.push(std::log(operand));
      else
        status_ = err;
      break;
    case '~':
      // Negate the operand
      stack_.push(operand * -1);
      break;
    case '&':
      // Duplicate the operand
      stack_.push(operand);
      break;
    default:
      status_ = err;  // Unknown function symbol
      break;
  }
}

bool CalculatorModel::isNumber(std::string::iterator &it) noexcept {
  return isdigit(*it) || *it == '.' || *it == ',' || *it == 'e' || *it == 'x' ||
         ((*it == '-' || *it == '+') &&
          (it != infix_.begin() && *(it - 1) == 'e'));
}

bool CalculatorModel::isUnary(std::string::iterator &it) noexcept {
  return (*it == '-' || *it == '+') &&
         (it == infix_.begin() || *(it - 1) == '(');
}

bool CalculatorModel::isBrackets(std::string::iterator &it) noexcept {
  return *it == '(' || *it == ')';
}

bool CalculatorModel::isOperators(std::string::iterator &it) noexcept {
  return *it == '-' || *it == '+' || *it == '*' || *it == '/' || *it == '%' ||
         *it == '^';
}

bool CalculatorModel::isFunction(std::string::iterator &it) noexcept {
  char ch = ChangeFunctions(it);
  return ch >= 65 && ch <= 73 && *it != ch;
}

bool CalculatorModel::isPow(std::string::iterator &it) noexcept {
  return *it == '^';
}

bool CalculatorModel::isValid() {
  if (infix_ == "") return status_ = err, false;
  int opened = 0, closed = 0;
  for (auto it = infix_.begin(); it != infix_.end(); ++it) {
    if (*it == '(')
      ++opened;
    else if (*it == ')')
      ++closed;
  }
  if (opened != closed) status_ = err;
  return opened == closed;
}

void CalculatorModel::NumberToPostfix(std::string::iterator &it) {
  while (isNumber(it)) postfix_.push_back(*it++);
  *it--;
  postfix_.push_back(' ');
}

void CalculatorModel::UnaryToPostFix(std::string::iterator &it) {
  char ch = *it == '+' ? '&' : '~';
  while (!stack_.empty() && Priority(ch) <= Priority(stack_.top()))
    PushBackPostfix();
  stack_.push(ch);
}

void CalculatorModel::BracketsToPostfix(std::string::iterator &it) {
  if (*it == '(')
    stack_.push(*it);
  else {
    while (!stack_.empty() && stack_.top() != '(') PushBackPostfix();
    if (!stack_.empty() && stack_.top() == '(')
      stack_.pop();
    else
      status_ = err;
  }
}

void CalculatorModel::PowToPostfix(std::string::iterator &it) {
  while (!stack_.empty() && Priority(*it) < Priority(stack_.top()))
    PushBackPostfix();
  stack_.push(*it);
}

void CalculatorModel::FunctionToPostfix(std::string::iterator &it) {
  char func_name = ChangeFunctions(it, true);
  while (!stack_.empty() && Priority(func_name) <= Priority(stack_.top()))
    PushBackPostfix();
  stack_.push(func_name);
}

char CalculatorModel::ChangeFunctions(std::string::iterator &it, bool iter) {
  std::vector<std::string> func = {"cos",  "sin",  "tan", "acos", "asin",
                                   "atan", "sqrt", "log", "ln"};
  for (int i = 0; i < (int)func.size(); ++i) {
    if (std::equal(it, it + func[i].size(), func[i].begin(), func[i].end())) {
      if (iter) it += func[i].length() - 1;
      return i + 65;
    }
  }
  return *it;
}

void CalculatorModel::zeroOut() {
  postfix_ = "";
  status_ = ok;
  result_ = 0;
  while (!stack_.empty()) stack_.pop();
}

int CalculatorModel::Priority(char ch) noexcept {
  if (ch == '+' || ch == '-')
    return 1;
  else if (ch == '*' || ch == '/')
    return 2;
  else if (ch == '^')
    return 3;
  else if ((ch >= 65 && ch <= 74) || ch == '%')
    return 4;
  else if (ch == '~' || ch == '&')
    return 5;
  else if (ch == '(')
    return -1;
  return 0;
}

void CalculatorModel::PushBackPostfix() {
  if (!stack_.empty()) {
    postfix_.push_back(stack_.top());
    stack_.pop();
    postfix_.push_back(' ');
  }
}

void CalculatorModel::SetInfix(const std::string &infix) noexcept {
  infix_ = infix;
}

void CalculatorModel::SetX(const long double x) noexcept { x_ = x; }
enum Status CalculatorModel::GetStatus() const { return status_; }
long double CalculatorModel::GetResult() const { return result_; }
}  // namespace s21
