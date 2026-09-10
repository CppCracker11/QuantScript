#pragma once
#include <string>

struct SourceLocation {
    int line = 1;
    int column = 1;
};

inline std::string locationString(const SourceLocation& loc) {
    return "line " + std::to_string(loc.line) + ", column " + std::to_string(loc.column);
}
