#include "lexer/Token.h"

std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::Strategy: return "STRATEGY";
        case TokenType::Buy: return "BUY";
        case TokenType::Sell: return "SELL";
        case TokenType::If: return "IF";
        case TokenType::StopLoss: return "STOPLOSS";
        case TokenType::TakeProfit: return "TAKEPROFIT";
        case TokenType::SMA: return "SMA";
        case TokenType::EMA: return "EMA";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Integer: return "INTEGER";
        case TokenType::Float: return "FLOAT";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Less: return "LESS";
        case TokenType::Greater: return "GREATER";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::NotEqual: return "NOT_EQUAL";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::Percentage: return "PERCENTAGE";
        case TokenType::Newline: return "NEWLINE";
        case TokenType::EndOfFile: return "EOF";
    }
    return "UNKNOWN";
}
