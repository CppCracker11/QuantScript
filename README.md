# \# QuantScript

# 

# QuantScript is a small domain-specific language compiler for expressing quantitative trading rules. It is designed as an undergraduate Compiler Design project rather than a trading platform.

# 

# \## Compiler Pipeline

# 

# 1\. Lexical analysis — handwritten lexer

# 2\. Syntax analysis — recursive-descent parser

# 3\. AST construction

# 4\. Semantic analysis + symbol table

# 5\. Quantitative rule representation

# 6\. LLVM IR generation + module verification

# 7\. LLVM O2 optimization

# 8\. Optional native object-code generation

# 

# \## Implemented Language

# 

# Example:

# 

# ```text

# STRATEGY MeanReversion

# BUY IF price < SMA(20)

# SELL IF price > SMA(20)

# STOPLOSS 2%

# TAKEPROFIT 5%

