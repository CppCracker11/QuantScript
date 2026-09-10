#include "semantic/SemanticAnalyzer.h"
#include "lexer/Lexer.h"
#include <cmath>

SymbolTable::SymbolTable() {
    symbols_.emplace("price", Symbol{"price",ValueType::Number,{1,1}});
    symbols_.emplace("volume", Symbol{"volume",ValueType::Number,{1,1}});
}
bool SymbolTable::contains(const std::string& n) const { return symbols_.find(n)!=symbols_.end(); }
const Symbol* SymbolTable::lookup(const std::string& n) const { auto it=symbols_.find(n); return it==symbols_.end()?nullptr:&it->second; }

void SemanticAnalyzer::analyze(const ProgramNode& p) {
    if (!p.strategy || p.strategy->name.empty()) throw CompilerError("Semantic Error","strategy name cannot be empty",p.location);
    for (const auto& s:p.strategy->statements) analyzeStatement(*s);
    if (!hasBuy_ && !hasSell_) throw CompilerError("Semantic Error","strategy must contain at least one BUY or SELL rule",p.location);
}

ValueType SemanticAnalyzer::analyzeExpression(const ExpressionNode& e) {
    if (dynamic_cast<const NumberNode*>(&e)) return ValueType::Number;
    if (auto id=dynamic_cast<const IdentifierNode*>(&e)) {
        if (!symbols_.contains(id->name)) throw CompilerError("Semantic Error","undefined identifier '"+id->name+"'",id->location);
        return ValueType::Number;
    }
    if (auto f=dynamic_cast<const FunctionCallNode*>(&e)) {
        if (f->name!="SMA" && f->name!="EMA") throw CompilerError("Semantic Error","unknown built-in function '"+f->name+"'",f->location);
        auto arg=dynamic_cast<const NumberNode*>(f->argument.get());
        if (!arg || std::floor(arg->value)!=arg->value || arg->value<=0) throw CompilerError("Semantic Error",f->name+" period must be a positive integer",f->location);
        return ValueType::Number;
    }
    if (auto b=dynamic_cast<const BinaryExpressionNode*>(&e)) {
        ValueType l=analyzeExpression(*b->left), r=analyzeExpression(*b->right);
        bool cmp=b->op==BinaryOp::Less||b->op==BinaryOp::Greater||b->op==BinaryOp::LessEqual||b->op==BinaryOp::GreaterEqual||b->op==BinaryOp::Equal||b->op==BinaryOp::NotEqual;
        if (l!=ValueType::Number||r!=ValueType::Number) throw CompilerError("Semantic Error","binary expression operands must be numeric",b->location);
        return cmp?ValueType::Boolean:ValueType::Number;
    }
    return ValueType::Unknown;
}

void SemanticAnalyzer::analyzeStatement(const StatementNode& s) {
    if (auto b=dynamic_cast<const BuyStatementNode*>(&s)) { if (analyzeExpression(*b->condition)!=ValueType::Boolean) throw CompilerError("Semantic Error","BUY IF requires a comparison expression",b->condition->location); hasBuy_=true; return; }
    if (auto x=dynamic_cast<const SellStatementNode*>(&s)) { if (analyzeExpression(*x->condition)!=ValueType::Boolean) throw CompilerError("Semantic Error","SELL IF requires a comparison expression",x->condition->location); hasSell_=true; return; }
    if (auto sl=dynamic_cast<const StopLossNode*>(&s)) { if (!(sl->value>0 && sl->value<=100)) throw CompilerError("Semantic Error","STOPLOSS must be greater than 0% and at most 100%",sl->location); return; }
    if (auto tp=dynamic_cast<const TakeProfitNode*>(&s)) { if (!(tp->value>0 && tp->value<=1000)) throw CompilerError("Semantic Error","TAKEPROFIT must be greater than 0% and at most 1000%",tp->location); return; }
}
