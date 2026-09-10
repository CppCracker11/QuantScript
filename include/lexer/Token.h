#pragma once
#include "common/SourceLocation.h"
#include <string>

enum class TokenType {
    Strategy, Buy, Sell, If, StopLoss, TakeProfit, SMA, EMA,
    Identifier, Integer, Float,
    Plus, Minus, Star, Slash,
    Less, Greater, LessEqual, GreaterEqual, EqualEqual, NotEqual,
    LeftParen, RightParen, Percentage,
    Newline, EndOfFile
};

struct Token {
    TokenType type;
    std::string lexeme;
    SourceLocation location;
};

std::string tokenTypeName(TokenType type);
