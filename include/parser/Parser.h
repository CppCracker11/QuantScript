#pragma once
#include "ast/AST.h"
#include "lexer/Token.h"
#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramNode> parse();
private:
    const Token& current() const;
    const Token& previous() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& message);
    void skipNewlines();
    std::unique_ptr<StrategyNode> parseStrategy();
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseComparison();
    std::unique_ptr<ExpressionNode> parseArithmetic();
    std::unique_ptr<ExpressionNode> parseTerm();
    std::unique_ptr<ExpressionNode> parseFactor();
    std::unique_ptr<ExpressionNode> parseFunctionCall();
    double parsePercentValue(const Token& keyword);
    std::vector<Token> tokens_;
    size_t current_ = 0;
};
