#pragma once
#include "lexer/Token.h"
#include <stdexcept>
#include <string>
#include <vector>

class CompilerError : public std::runtime_error {
public:
    CompilerError(const std::string& kind, const std::string& message, SourceLocation loc);
    const std::string& kind() const { return kind_; }
    const SourceLocation& location() const { return location_; }
private:
    std::string kind_;
    SourceLocation location_;
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();
private:
    char peek(int offset = 0) const;
    char advance();
    bool match(char c);
    void add(TokenType type, const std::string& lexeme, SourceLocation loc);
    void scanNumber();
    void scanIdentifier();
    std::string source_;
    std::vector<Token> tokens_;
    size_t current_ = 0;
    int line_ = 1;
    int column_ = 1;
    int tokenColumn_ = 1;
};
