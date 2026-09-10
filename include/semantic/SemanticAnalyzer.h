#pragma once
#include "ast/AST.h"
#include <string>
#include <unordered_map>

struct Symbol { std::string name; ValueType type; SourceLocation location; };

class SymbolTable {
public:
    SymbolTable();
    bool contains(const std::string& name) const;
    const Symbol* lookup(const std::string& name) const;
private:
    std::unordered_map<std::string, Symbol> symbols_;
};

class SemanticAnalyzer {
public:
    void analyze(const ProgramNode& program);
private:
    ValueType analyzeExpression(const ExpressionNode& expr);
    void analyzeStatement(const StatementNode& stmt);
    SymbolTable symbols_;
    bool hasBuy_ = false;
    bool hasSell_ = false;
};
