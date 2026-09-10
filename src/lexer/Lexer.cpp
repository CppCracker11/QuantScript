#include "lexer/Lexer.h"
#include <cctype>
#include <unordered_map>

CompilerError::CompilerError(const std::string& kind, const std::string& message, SourceLocation loc)
    : std::runtime_error(kind + " at " + locationString(loc) + ": " + message), kind_(kind), location_(loc) {}

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek(int offset) const {
    size_t p = current_ + static_cast<size_t>(offset);
    return p < source_.size() ? source_[p] : '\0';
}

char Lexer::advance() {
    char c = peek();
    if (c == '\0') return c;
    ++current_;
    if (c == '\n') { ++line_; column_ = 1; }
    else ++column_;
    return c;
}

bool Lexer::match(char c) {
    if (peek() != c) return false;
    advance();
    return true;
}

void Lexer::add(TokenType type, const std::string& lexeme, SourceLocation loc) {
    tokens_.push_back({type, lexeme, loc});
}

void Lexer::scanNumber() {
    SourceLocation loc{line_, tokenColumn_};
    size_t start = current_;
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    bool floating = false;
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        floating = true;
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    } else if (peek() == '.') {
        throw CompilerError("Lexical Error", "malformed number literal", loc);
    }
    std::string text = source_.substr(start, current_ - start);
    add(floating ? TokenType::Float : TokenType::Integer, text, loc);
}

void Lexer::scanIdentifier() {
    SourceLocation loc{line_, tokenColumn_};
    size_t start = current_;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') advance();
    std::string text = source_.substr(start, current_ - start);
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"STRATEGY", TokenType::Strategy}, {"BUY", TokenType::Buy}, {"SELL", TokenType::Sell},
        {"IF", TokenType::If}, {"STOPLOSS", TokenType::StopLoss}, {"TAKEPROFIT", TokenType::TakeProfit},
        {"SMA", TokenType::SMA}, {"EMA", TokenType::EMA}
    };
    auto it = keywords.find(text);
    add(it == keywords.end() ? TokenType::Identifier : it->second, text, loc);
}

std::vector<Token> Lexer::tokenize() {
    while (current_ < source_.size()) {
        char c = peek();
        tokenColumn_ = column_;
        if (c == ' ' || c == '\t' || c == '\r') { advance(); continue; }
        if (c == '#') { while (peek() && peek() != '\n') advance(); continue; }
        if (c == '\n') { SourceLocation l{line_, column_}; advance(); add(TokenType::Newline, "\\n", l); continue; }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') { scanIdentifier(); continue; }
        if (std::isdigit(static_cast<unsigned char>(c))) { scanNumber(); continue; }
        SourceLocation loc{line_, column_};
        switch (c) {
            case '+': advance(); add(TokenType::Plus, "+", loc); break;
            case '-': advance(); add(TokenType::Minus, "-", loc); break;
            case '*': advance(); add(TokenType::Star, "*", loc); break;
            case '/': advance(); add(TokenType::Slash, "/", loc); break;
            case '(': advance(); add(TokenType::LeftParen, "(", loc); break;
            case ')': advance(); add(TokenType::RightParen, ")", loc); break;
            case '%': advance(); add(TokenType::Percentage, "%", loc); break;
            case '<': advance(); if (match('=')) add(TokenType::LessEqual, "<=", loc); else add(TokenType::Less, "<", loc); break;
            case '>': advance(); if (match('=')) add(TokenType::GreaterEqual, ">=", loc); else add(TokenType::Greater, ">", loc); break;
            case '=': advance(); if (match('=')) add(TokenType::EqualEqual, "==", loc); else throw CompilerError("Lexical Error", "single '=' is not valid; use '==' for comparison", loc); break;
            case '!': advance(); if (match('=')) add(TokenType::NotEqual, "!=", loc); else throw CompilerError("Lexical Error", "unexpected '!'; use '!=' for comparison", loc); break;
            default: throw CompilerError("Lexical Error", std::string("unknown character '") + c + "'", loc);
        }
    }
    tokens_.push_back({TokenType::EndOfFile, "", {line_, column_}});
    return tokens_;
}
