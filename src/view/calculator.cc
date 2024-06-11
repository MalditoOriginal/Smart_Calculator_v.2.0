#include "calculator.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <algorithm>

#include "ui_calculator.h"

Calculator::Calculator(QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::Calculator) {
  ui_->setupUi(this);
  setWindowTitle("SmartCalculator");
  setFixedSize(width(), height());
  SetTextToOut("0");
  InitializeConnections();
}

Calculator::~Calculator() { delete ui_; }

void Calculator::DigitsNum() {
  QString current_text = ui_->Labe_out->toPlainText();
  QPushButton *button = static_cast<QPushButton *>(sender());
  if (isNothingStr(current_text) || current_text == "+") current_text = "";
  if (button->text() == "X")
    SetTextX(current_text);
  else if (button->text() == "e")
    SetTextE(current_text);
  else if (button->text() == ".")
    SetTextDot(current_text);
  else
    SetTextDigits(current_text, button->text());
}

void Calculator::Function() {
  QString current_text = ui_->Labe_out->toPlainText();
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  QString button_text = button->text();

  if (button_text == "√a") button_text = "sqrt";

  // Clear current text if it's nothing
  if (isNothingStr(current_text)) current_text.clear();

  // Determine appropriate action based on current text
  if (current_text.isEmpty() || current_text.endsWith('(') ||
      current_text.endsWith('x'))
    SetTextToOut(current_text + button_text + "(");
  else if (current_text.back().isDigit() ||
           isOperator(current_text.back().toLatin1()))
    SetTextToOut(current_text + '*' + button_text + "(");
  else
    SetTextToOut(current_text + "(" + button_text + "(");
}

void Calculator::Operators() {
  QString current_text = ui_->Labe_out->toPlainText();
  QPushButton *button = static_cast<QPushButton *>(sender());
  QString button_text = button->text();

  if (button_text == "×")
    button_text = '*';
  else if (button_text == "÷")
    button_text = '/';
  else if (button_text == "mod")
    button_text = '%';
  else if (button_text == "a^b")
    button_text = '^';
  SetTextOperator(current_text, button_text);
}

void Calculator::Brackets() {
  QString current_text = ui_->Labe_out->toPlainText();
  QPushButton *button = static_cast<QPushButton *>(sender());

  if (isNothingStr(current_text) || current_text == "+") current_text.clear();
  if (button->text() == "(")
    SetTextOpenBrackets(current_text);
  else if (button->text() == ")")
    SetTextClosedBrackets(current_text);
}

void Calculator::Delete() {
  QString current_text = ui_->Labe_out->toPlainText();
  QPushButton *button = static_cast<QPushButton *>(sender());

  if (button->text() == "AC")
    DeleteText();
  else if (button->text() == "C")
    ChopText(current_text);
}

void Calculator::ShowRes() {
  QPushButton *button = static_cast<QPushButton *>(sender());
  if (button->text() == "=")
    Calculate();
  else if (button->text() == "PLOT")
    PlotGraph();
}

void Calculator::GraphWindow() {
  QPushButton *button_graph = ui_->PushButton_graph;
  QPushButton *button_equals = ui_->PushButton_equals;

  bool graphPressed = button_graph->property("isPressed").toBool();

  // Toggle graph button style and text
  button_graph->setStyleSheet(graphPressed ? kAdditionalWindowStyleNormal_
                                           : kAdditionalWindowStylePressed_);
  button_equals->setText(graphPressed ? "=" : "PLOT");
  button_equals->setStyleSheet(graphPressed ? kEqualsButtonStyle_
                                            : kPlotButtonStyle_);

  if (graphPressed) {  // If graph button is pressed
    // Close graph window if it's open
    if (graph_window_ && graph_window_->isVisible()) {
      graph_window_->close();
      disconnect(this, &Calculator::Plot, graph_window_, &Graph::Plot);
      delete graph_window_;
      graph_window_ = nullptr;
    }
  } else {  // If graph button is not pressed
    // Create and show graph window if it's not already visible
    if (!graph_window_) {
      graph_window_ = new Graph(this);
      graph_window_->setGeometry(x() + width(), y() + 28, 400, 570);
      graph_window_->show();
      connect(this, &Calculator::Plot, graph_window_, &Graph::Plot);
    } else if (!graph_window_->isVisible()) {
      graph_window_->show();
      connect(this, &Calculator::Plot, graph_window_, &Graph::Plot);
    }
  }

  // Toggle graph button property
  button_graph->setProperty("isPressed", !graphPressed);
}

void Calculator::CreditWindow() {
  QPushButton *button_cred = ui_->PushButton_cred;
  QPushButton *button_dep = ui_->PushButton_dep;

  bool credPressed = button_cred->property("isPressed").toBool();
  bool depPressed = button_dep->property("isPressed").toBool();

  // Close credit window if it's already open and credit button is pressed again
  if (credPressed && credit_window_ && credit_window_->isVisible()) {
    credit_window_->close();
    delete credit_window_;
    credit_window_ = nullptr;
  } else {
    // Set style for credit button
    button_cred->setStyleSheet(credPressed ? kAdditionalWindowStyleNormal_
                                           : kAdditionalWindowStylePressed_);

    // Create and show credit window if credit button is pressed
    if (!credPressed) {
      credit_window_ = new Credit(this);
      credit_window_->setGeometry(this->x() - 450, this->y() + 100, 450, 285);
      credit_window_->show();
    }

    // Close deposit window if it's open and deposit button is pressed
    if (depPressed && deposit_window_ && deposit_window_->isVisible()) {
      deposit_window_->close();
      delete deposit_window_;
      deposit_window_ = nullptr;
      // Reset deposit button properties and style
      button_dep->setProperty("isPressed", false);
      button_dep->setStyleSheet(kAdditionalWindowStyleNormal_);
    }
  }

  // Toggle credit button property
  button_cred->setProperty("isPressed", !credPressed);
}

void Calculator::DepositWindow() {
  QPushButton *button_dep = ui_->PushButton_dep,
              *button_cred = ui_->PushButton_cred;

  bool depPressed = button_dep->property("isPressed").toBool();
  bool credPressed = button_cred->property("isPressed").toBool();

  // Close deposit window if it's already open and deposit button is pressed
  // again
  if (depPressed && deposit_window_ && deposit_window_->isVisible()) {
    deposit_window_->close();
    delete deposit_window_;
    deposit_window_ = nullptr;
  } else {
    // Set style for deposit button
    button_dep->setStyleSheet(depPressed ? kAdditionalWindowStyleNormal_
                                         : kAdditionalWindowStylePressed_);

    // Create and show deposit window if deposit button is pressed
    if (!depPressed) {
      deposit_window_ = new Deposit(this);
      deposit_window_->setGeometry(this->x() - 450, this->y() + 100, 450, 465);
      deposit_window_->show();
    }

    // Close credit window if it's open and credit button is pressed
    if (credPressed && credit_window_ && credit_window_->isVisible()) {
      credit_window_->close();
      delete credit_window_;
      credit_window_ = nullptr;
      // Reset credit button properties and style
      button_cred->setProperty("isPressed", false);
      button_cred->setStyleSheet(kAdditionalWindowStyleNormal_);
    }
  }

  // Toggle deposit button property
  button_dep->setProperty("isPressed", !depPressed);
}

void Calculator::InitializeConnections() {
  QPushButton *buttons[] = {
      ui_->PushButton_0,  ui_->PushButton_1, ui_->PushButton_2,
      ui_->PushButton_3,  ui_->PushButton_4, ui_->PushButton_5,
      ui_->PushButton_6,  ui_->PushButton_7, ui_->PushButton_8,
      ui_->PushButton_9,  ui_->PushButton_x, ui_->PushButton_e,
      ui_->PushButton_dot};
  for (QPushButton *button : buttons)
    connect(button, SIGNAL(clicked()), this, SLOT(DigitsNum()));

  QPushButton *functionButtons[] = {
      ui_->PushButton_cos,  ui_->PushButton_tan,  ui_->PushButton_sin,
      ui_->PushButton_atan, ui_->PushButton_asin, ui_->PushButton_acos,
      ui_->PushButton_log,  ui_->PushButton_ln,   ui_->PushButton_sqrt};
  for (QPushButton *button : functionButtons)
    connect(button, SIGNAL(clicked()), this, SLOT(Function()));

  QPushButton *operatorButtons[] = {ui_->PushButton_plus, ui_->PushButton_minus,
                                    ui_->PushButton_mult, ui_->PushButton_div,
                                    ui_->PushButton_exp,  ui_->pushButton_mod};
  for (QPushButton *button : operatorButtons)
    connect(button, SIGNAL(clicked()), this, SLOT(Operators()));

  connect(ui_->PushButton_bracket1, SIGNAL(clicked()), this, SLOT(Brackets()));
  connect(ui_->PushButton_bracket2, SIGNAL(clicked()), this, SLOT(Brackets()));
  connect(ui_->PushButton_del, SIGNAL(clicked()), this, SLOT(Delete()));
  connect(ui_->PushButton_all_del, SIGNAL(clicked()), this, SLOT(Delete()));
  connect(ui_->PushButton_equals, SIGNAL(clicked()), this, SLOT(ShowRes()));
  connect(ui_->PushButton_graph, SIGNAL(clicked()), this, SLOT(GraphWindow()));
  connect(ui_->PushButton_cred, SIGNAL(clicked()), this, SLOT(CreditWindow()));
  connect(ui_->PushButton_dep, SIGNAL(clicked()), this, SLOT(DepositWindow()));
}

QString Calculator::GetLastNumber(QString current_text) {
  int last_number_start = -1, text_length = current_text.length();

  for (int i = text_length - 1; i >= 0; i--) {
    QChar ch = current_text[i];
    last_number_start = (ch.isDigit() || ch == '.' || ch == 'e') ? i
                        : (ch == '-' && i > 0 && current_text[i - 1] == 'e')
                            ? i
                            : -1;
    if (last_number_start == -1) break;
  }

  return (last_number_start >= 0) ? current_text.mid(last_number_start) : "";
}

bool Calculator::isOperator(char ch) {
  std::array<char, 7> operators = {'+', '-', '*', '/', '^', '%', '^'};
  return std::any_of(operators.begin(), operators.end(),
                     [ch](char c) { return c == ch; });
}

bool Calculator::isNothingStr(QString current_text) {
  return current_text.isEmpty() || current_text == "0" ||
         current_text == "Error" || current_text.endsWith("\u221E");
}

QString Calculator::AddCloseBrackets() {
  QString current_text = ui_->Labe_out->toPlainText();
  while (current_text.count('(') > current_text.count(')') &&
         current_text.back() != '(')
    current_text += ')';
  return current_text;
}

void Calculator::SetTextToOut(QString string) {
  ui_->Labe_out->setText(string);
  ui_->Labe_out->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  QTextCursor cursor = ui_->Labe_out->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui_->Labe_out->setTextCursor(cursor);
  ui_->Labe_out->setWordWrapMode(QTextOption::NoWrap);
  ui_->Labe_out->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

QString Calculator::ChopZero(long double result) {
  QString res_string = QString::number(result, 'f', 6);
  while (res_string.endsWith('0')) res_string.chop(1);
  if (res_string.endsWith('.') || res_string.endsWith(',')) res_string.chop(1);
  return res_string;
}

void Calculator::SetTextDigits(QString current_text, QString button_text) {
  if (isNothingStr(current_text) || current_text == "+") current_text = "";
  if (current_text.endsWith(')') || current_text.endsWith('x')) {
    SetTextToOut(current_text + "*" + button_text);
  } else if (button_text == "0" && current_text.endsWith('(')) {
    SetTextToOut(current_text + button_text + ".");
  } else if ((current_text.endsWith('+') && current_text.length() >= 2 &&
              *(current_text.rbegin() + 1) == QChar('(')) ||
             current_text == "+") {
    current_text.chop(1);
    SetTextToOut(current_text + button_text);
  } else {
    SetTextToOut(current_text + button_text);
  }
}

void Calculator::SetTextDot(QString current_text) {
  if (current_text.isEmpty() || isOperator(current_text.back().toLatin1()) ||
      current_text.endsWith('('))
    SetTextToOut(current_text + "0.");
  else {
    QString last_number = GetLastNumber(current_text);
    if (!last_number.contains('.') && !last_number.contains('e'))
      SetTextToOut(current_text + ".");
  }
}

void Calculator::SetTextOperator(QString current_text, QString button_text) {
  if (isNothingStr(current_text))
    current_text = "";
  else if (isOperator(current_text.back().toLatin1()) ||
           current_text.back() == '.')
    current_text.chop(1);

  if (current_text.isEmpty() || current_text.endsWith('(') ||
      current_text.endsWith('e')) {
    if (button_text == "+" || button_text == "-")
      SetTextToOut(current_text + button_text);
  } else {
    if (current_text.back().isDigit() || current_text.endsWith('x') ||
        current_text.endsWith(')'))
      SetTextToOut(current_text + button_text);
  }
}

void Calculator::SetTextOpenBrackets(QString current_text) {
  if (isNothingStr(current_text) || current_text == "+") current_text = "";
  if (current_text.isEmpty()) {
    SetTextToOut("(");
  } else if (isOperator(current_text.back().toLatin1()) ||
             current_text.back() == '(') {
    SetTextToOut(current_text + '(');
  } else if (!(current_text.endsWith('('))) {
    SetTextToOut(current_text + "*(");
  }
}

void Calculator::SetTextClosedBrackets(QString current_text) {
  if (current_text.count('(') > current_text.count(')') &&
      !current_text.endsWith('(') &&
      (current_text.back().isDigit() || current_text.endsWith('x') ||
       current_text.endsWith(')')))
    SetTextToOut(current_text + ')');
}

void Calculator::SetTextX(QString current_text) {
  if (isNothingStr(current_text) || current_text == "+") current_text = "";
  if (current_text.isEmpty()) {
    SetTextToOut("x");
  } else if (current_text.back().isDigit() || current_text.back() == ')' ||
             current_text.back() == 'x') {
    SetTextToOut(current_text + "*x");
  } else if (!GetLastNumber(current_text).contains('e'))
    SetTextToOut(current_text + "x");
}

void Calculator::SetTextE(QString current_text) {
  QString last_number = GetLastNumber(current_text);
  if (!current_text.isEmpty() && current_text.back().isDigit() &&
      !last_number.contains('e'))
    SetTextToOut(current_text + "e");
}

void Calculator::DeleteText() {
  ui_->xValue->clear();
  SetTextToOut("0");
}

void Calculator::ChopText(QString current_text) {
  if (current_text.length() > 1) {
    current_text.chop(1);
    while (!current_text.isEmpty() &&
           ((current_text.back().toLatin1() >= 97 &&
             current_text.back().toLatin1() <= 122 &&
             current_text.back().toLatin1() != 'x') ||
            current_text.endsWith('E')))
      current_text.chop(1);
    SetTextToOut(current_text);
  } else
    SetTextToOut("0");
}

void Calculator::Calculate() {
  QString current_text = AddCloseBrackets();
  bool x_value_convert;
  long double x_value = ui_->xValue->toPlainText().toDouble(&x_value_convert);
  int count_x = current_text.count('x');
  if ((count_x == 0 && !x_value_convert) || (count_x > 0 && x_value_convert) ||
      current_text.length() <= 255) {
    calculator_controller_.SetExpersion(current_text.toStdString(), x_value);
    calculator_controller_.Calculate();
    if (calculator_controller_.GetStatus() == s21::ok) {
      long double result = calculator_controller_.GetResult();
      SetTextToOut(ChopZero(result));
      if (ui_->Labe_out->toPlainText().startsWith("nan"))
        SetTextToOut("Error");
      else if (ui_->Labe_out->toPlainText().endsWith("-inf"))
        SetTextToOut("-\u221E");
      else if (ui_->Labe_out->toPlainText().endsWith("inf"))
        SetTextToOut("\u221E");
    } else
      SetTextToOut("Error");
  } else if (count_x > 0 && !x_value_convert) {
    QMessageBox::information(this, "ERROR", "Set value of x correctly");
  } else
    SetTextToOut("Error");
}

void Calculator::PlotGraph() {
  QString current_text = AddCloseBrackets();
  SetTextToOut(current_text);
  QString x_value_str = ui_->xValue->toPlainText();
  emit Plot(current_text, x_value_str);
}

void Calculator::keyPressEvent(QKeyEvent *event) {
  QString current_text = ui_->Labe_out->toPlainText();
  int key = event->key();
  if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    QString key_text = QString::number(key - Qt::Key_0);
    SetTextDigits(current_text, key_text);
  } else if (key == Qt::Key_Period || key == Qt::Key_Comma) {
    SetTextDot(current_text);
  } else if (key == Qt::Key_X) {
    SetTextX(current_text);
  } else if (key == Qt::Key_E) {
    SetTextE(current_text);
  } else if (key == Qt::Key_Plus) {
    SetTextOperator(current_text, "+");
  } else if (key == Qt::Key_Minus) {
    SetTextOperator(current_text, "-");
  } else if (key == Qt::Key_Asterisk) {
    SetTextOperator(current_text, "*");
  } else if (key == Qt::Key_Slash) {
    SetTextOperator(current_text, "/");
  } else if (key == Qt::Key_Percent) {
    SetTextOperator(current_text, "%");
  } else if (key == Qt::Key_AsciiCircum) {
    SetTextOperator(current_text, "^");
  } else if (key == Qt::Key_ParenLeft) {
    SetTextOpenBrackets(current_text);
  } else if (key == Qt::Key_ParenRight) {
    SetTextClosedBrackets(current_text);
  } else if (key == Qt::Key_Delete) {
    DeleteText();
  } else if (key == Qt::Key_Backspace) {
    ChopText(current_text);
  } else if (key == Qt::Key_Enter || key == Qt::Key_Return) {
    if (ui_->PushButton_equals->text() == "=")
      Calculate();
    else if (ui_->PushButton_equals->text() == "PLOT")
      PlotGraph();
  }
}

void Calculator::moveEvent(QMoveEvent *event) {
  QPushButton *button_graph = ui_->PushButton_graph;
  QPushButton *button_cred = ui_->PushButton_cred;
  QPushButton *button_dep = ui_->PushButton_dep;
  QWidget::moveEvent(event);
  QPoint mainPos = pos();  // Get the position of the main calculator window

  if (button_graph->property("isPressed").toBool() && graph_window_ &&
      graph_window_->isVisible()) {
    graph_window_->move(mainPos.x() + 290, mainPos.y());
  }

  if ((button_cred->property("isPressed").toBool() ||
       button_dep->property("isPressed").toBool()) &&
      ((credit_window_ && credit_window_->isVisible()) ||
       (deposit_window_ && deposit_window_->isVisible()))) {
    // Move credit_window_ and deposit_window_ to a new position relative to the
    // main calculator window
    int xOffset = -450;
    int yOffset = 72;
    credit_window_->move(mainPos.x() + xOffset, mainPos.y() + yOffset);
    deposit_window_->move(mainPos.x() + xOffset, mainPos.y() + yOffset);
  }
}