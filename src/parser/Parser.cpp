#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include <cstdlib>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}
const Token& Parser::current() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_-1]; }
bool Parser::check(TokenType t) const { return current().type == t; }
bool Parser::match(TokenType t) { if (!check(t)) return false; ++current_; return true; }
const Token& Parser::consume(TokenType t, const std::string& msg) { if (!check(t)) throw CompilerError("Syntax Error", msg, current().location); return tokens_[current_++]; }
void Parser::skipNewlines() { while (match(TokenType::Newline)) {} }

std::unique_ptr<ProgramNode> Parser::parse() {
    skipNewlines();
    auto strategy = parseStrategy();
    skipNewlines();
    if (!check(TokenType::EndOfFile)) throw CompilerError("Syntax Error", "unexpected token after strategy", current().location);
    auto p = std::make_unique<ProgramNode>(strategy->location);
    p->strategy = std::move(strategy);
    return p;
}

std::unique_ptr<StrategyNode> Parser::parseStrategy() {
    const Token& k = consume(TokenType::Strategy, "expected STRATEGY at the beginning of the program");
    const Token& name = consume(TokenType::Identifier, "expected strategy name after STRATEGY");
    auto s = std::make_unique<StrategyNode>(name.lexeme, k.location);
    skipNewlines();
    while (!check(TokenType::EndOfFile)) {
        s->statements.push_back(parseStatement());
        skipNewlines();
    }
    return s;
}

std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (match(TokenType::Buy)) {
        Token k = previous(); consume(TokenType::If, "expected IF after BUY");
        auto e = parseExpression(); return std::make_unique<BuyStatementNode>(std::move(e), k.location);
    }
    if (match(TokenType::Sell)) {
        Token k = previous(); consume(TokenType::If, "expected IF after SELL");
        auto e = parseExpression(); return std::make_unique<SellStatementNode>(std::move(e), k.location);
    }
    if (match(TokenType::StopLoss)) { Token k=previous(); double v=parsePercentValue(k); return std::make_unique<StopLossNode>(v,k.location); }
    if (match(TokenType::TakeProfit)) { Token k=previous(); double v=parsePercentValue(k); return std::make_unique<TakeProfitNode>(v,k.location); }
    throw CompilerError("Syntax Error", "expected BUY, SELL, STOPLOSS, or TAKEPROFIT", current().location);
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() { return parseComparison(); }

std::unique_ptr<ExpressionNode> Parser::parseComparison() {
    auto left = parseArithmetic();
    if (check(TokenType::Less) || check(TokenType::Greater) || check(TokenType::LessEqual) || check(TokenType::GreaterEqual) || check(TokenType::EqualEqual) || check(TokenType::NotEqual)) {
        Token op = current(); ++current_;
        auto right = parseArithmetic();
        BinaryOp b;
        switch(op.type) { case TokenType::Less:b=BinaryOp::Less;break; case TokenType::Greater:b=BinaryOp::Greater;break; case TokenType::LessEqual:b=BinaryOp::LessEqual;break; case TokenType::GreaterEqual:b=BinaryOp::GreaterEqual;break; case TokenType::EqualEqual:b=BinaryOp::Equal;break; default:b=BinaryOp::NotEqual; }
        left = std::make_unique<BinaryExpressionNode>(b,std::move(left),std::move(right),op.location);
        if (check(TokenType::Less)||check(TokenType::Greater)||check(TokenType::LessEqual)||check(TokenType::GreaterEqual)||check(TokenType::EqualEqual)||check(TokenType::NotEqual)) throw CompilerError("Syntax Error","chained comparisons are not supported",current().location);
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseArithmetic() {
    auto left=parseTerm();
    while (check(TokenType::Plus)||check(TokenType::Minus)) { Token op=current();++current_; auto right=parseTerm(); left=std::make_unique<BinaryExpressionNode>(op.type==TokenType::Plus?BinaryOp::Add:BinaryOp::Subtract,std::move(left),std::move(right),op.location); }
    return left;
}
std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto left=parseFactor();
    while (check(TokenType::Star)||check(TokenType::Slash)) { Token op=current();++current_; auto right=parseFactor(); left=std::make_unique<BinaryExpressionNode>(op.type==TokenType::Star?BinaryOp::Multiply:BinaryOp::Divide,std::move(left),std::move(right),op.location); }
    return left;
}
std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    if (match(TokenType::Integer)||match(TokenType::Float)) return std::make_unique<NumberNode>(std::stod(previous().lexeme),previous().location);
    if (match(TokenType::Identifier)) return std::make_unique<IdentifierNode>(previous().lexeme,previous().location);
    if (check(TokenType::SMA)||check(TokenType::EMA)) return parseFunctionCall();
    if (match(TokenType::Minus)) { Token m=previous(); auto n=parseFactor(); auto zero=std::make_unique<NumberNode>(0.0,m.location); return std::make_unique<BinaryExpressionNode>(BinaryOp::Subtract,std::move(zero),std::move(n),m.location); }
    if (match(TokenType::LeftParen)) { auto e=parseExpression(); consume(TokenType::RightParen,"expected ')' after expression"); return e; }
    throw CompilerError("Syntax Error","expected expression",current().location);
}
std::unique_ptr<ExpressionNode> Parser::parseFunctionCall() {
    Token fn=current(); ++current_; consume(TokenType::LeftParen,"expected '(' after indicator name");
    auto arg=parseExpression();
    consume(TokenType::RightParen,"expected ')' after indicator argument");
    return std::make_unique<FunctionCallNode>(fn.lexeme,std::move(arg),fn.location);
}

double Parser::parsePercentValue(const Token& keyword) {
    if (!check(TokenType::Integer) && !check(TokenType::Float)) throw CompilerError("Syntax Error","expected numeric percentage after "+keyword.lexeme,current().location);
    double v=std::stod(current().lexeme); ++current_;
    consume(TokenType::Percentage,"expected '%' after percentage value");
    return v;
}
