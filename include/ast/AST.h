#pragma once
#include "common/SourceLocation.h"
#include <memory>
#include <string>
#include <vector>

enum class ValueType { Number, Boolean, Percentage, Unknown };

enum class BinaryOp { Add, Subtract, Multiply, Divide, Less, Greater, LessEqual, GreaterEqual, Equal, NotEqual };

class ASTNode {
public:
    explicit ASTNode(SourceLocation loc) : location(loc) {}
    virtual ~ASTNode() = default;
    SourceLocation location;
    virtual void print(std::ostream& out, int indent = 0) const = 0;
};

class ExpressionNode : public ASTNode { using ASTNode::ASTNode; };
class StatementNode : public ASTNode { using ASTNode::ASTNode; };

class NumberNode : public ExpressionNode {
public:
    NumberNode(double value, SourceLocation loc) : ExpressionNode(loc), value(value) {}
    double value;
    void print(std::ostream& out, int indent) const override;
};

class IdentifierNode : public ExpressionNode {
public:
    IdentifierNode(std::string name, SourceLocation loc) : ExpressionNode(loc), name(std::move(name)) {}
    std::string name;
    void print(std::ostream& out, int indent) const override;
};

class FunctionCallNode : public ExpressionNode {
public:
    FunctionCallNode(std::string name, std::unique_ptr<ExpressionNode> arg, SourceLocation loc)
        : ExpressionNode(loc), name(std::move(name)), argument(std::move(arg)) {}
    std::string name;
    std::unique_ptr<ExpressionNode> argument;
    void print(std::ostream& out, int indent) const override;
};

class BinaryExpressionNode : public ExpressionNode {
public:
    BinaryExpressionNode(BinaryOp op, std::unique_ptr<ExpressionNode> lhs, std::unique_ptr<ExpressionNode> rhs, SourceLocation loc)
        : ExpressionNode(loc), op(op), left(std::move(lhs)), right(std::move(rhs)) {}
    BinaryOp op;
    std::unique_ptr<ExpressionNode> left, right;
    void print(std::ostream& out, int indent) const override;
};

class BuyStatementNode : public StatementNode {
public:
    BuyStatementNode(std::unique_ptr<ExpressionNode> condition, SourceLocation loc) : StatementNode(loc), condition(std::move(condition)) {}
    std::unique_ptr<ExpressionNode> condition;
    void print(std::ostream& out, int indent) const override;
};

class SellStatementNode : public StatementNode {
public:
    SellStatementNode(std::unique_ptr<ExpressionNode> condition, SourceLocation loc) : StatementNode(loc), condition(std::move(condition)) {}
    std::unique_ptr<ExpressionNode> condition;
    void print(std::ostream& out, int indent) const override;
};

class StopLossNode : public StatementNode {
public:
    StopLossNode(double value, SourceLocation loc) : StatementNode(loc), value(value) {}
    double value;
    void print(std::ostream& out, int indent) const override;
};

class TakeProfitNode : public StatementNode {
public:
    TakeProfitNode(double value, SourceLocation loc) : StatementNode(loc), value(value) {}
    double value;
    void print(std::ostream& out, int indent) const override;
};

class StrategyNode : public ASTNode {
public:
    StrategyNode(std::string name, SourceLocation loc) : ASTNode(loc), name(std::move(name)) {}
    std::string name;
    std::vector<std::unique_ptr<StatementNode>> statements;
    void print(std::ostream& out, int indent = 0) const override;
};

class ProgramNode : public ASTNode {
public:
    explicit ProgramNode(SourceLocation loc) : ASTNode(loc) {}
    std::unique_ptr<StrategyNode> strategy;
    void print(std::ostream& out, int indent = 0) const override;
};

std::string binaryOpName(BinaryOp op);
