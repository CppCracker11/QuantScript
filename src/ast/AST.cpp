#include "ast/AST.h"
#include <iomanip>
#include <ostream>

static void pad(std::ostream& out, int n) { for (int i = 0; i < n; ++i) out << "  "; }
std::string binaryOpName(BinaryOp op) {
    switch (op) { case BinaryOp::Add:return "+"; case BinaryOp::Subtract:return "-"; case BinaryOp::Multiply:return "*"; case BinaryOp::Divide:return "/"; case BinaryOp::Less:return "<"; case BinaryOp::Greater:return ">"; case BinaryOp::LessEqual:return "<="; case BinaryOp::GreaterEqual:return ">="; case BinaryOp::Equal:return "=="; case BinaryOp::NotEqual:return "!="; }
    return "?";
}
void NumberNode::print(std::ostream& o, int i) const { pad(o,i); o << "Number(" << std::setprecision(12) << value << ")\n"; }
void IdentifierNode::print(std::ostream& o, int i) const { pad(o,i); o << "Identifier(" << name << ")\n"; }
void FunctionCallNode::print(std::ostream& o, int i) const { pad(o,i); o << "FunctionCall(" << name << ")\n"; argument->print(o,i+1); }
void BinaryExpressionNode::print(std::ostream& o, int i) const { pad(o,i); o << "Binary(" << binaryOpName(op) << ")\n"; left->print(o,i+1); right->print(o,i+1); }
void BuyStatementNode::print(std::ostream& o, int i) const { pad(o,i); o << "BUY\n"; condition->print(o,i+1); }
void SellStatementNode::print(std::ostream& o, int i) const { pad(o,i); o << "SELL\n"; condition->print(o,i+1); }
void StopLossNode::print(std::ostream& o, int i) const { pad(o,i); o << "STOPLOSS(" << value << "%)\n"; }
void TakeProfitNode::print(std::ostream& o, int i) const { pad(o,i); o << "TAKEPROFIT(" << value << "%)\n"; }
void StrategyNode::print(std::ostream& o, int i) const { pad(o,i); o << "Strategy(" << name << ")\n"; for (auto& s: statements) s->print(o,i+1); }
void ProgramNode::print(std::ostream& o, int i) const { pad(o,i); o << "Program\n"; if (strategy) strategy->print(o,i+1); }
