# QuantScript Architecture

```text
QuantScript source
       |
       v
     Lexer --------> Tokens
       |
       v
     Parser -------> AST
       |
       v
Semantic Analyzer -> Symbol checks + type checks
       |
       v
LLVM IR Generator -> LLVM Module
       |
       v
LLVM Optimizer ---> optimized LLVM IR
       |
       v
LLVM Target ------> native object file (optional)
```

## Lexer
The handwritten lexer converts source characters into typed tokens and records line/column positions. It reports unknown characters instead of silently skipping them.

## Parser
A recursive-descent parser consumes tokens according to the grammar and builds semantic AST nodes. Operator precedence is represented by separate expression parsing functions.

## AST
The AST removes punctuation-only syntax and represents strategy declarations, trading statements, arithmetic/comparison expressions, identifiers, numbers, and indicator calls.

## Semantic Analyzer
The semantic phase owns a small symbol table containing the built-in numeric identifiers `price` and `volume`. It validates indicator names/periods, expression types, strategy rules, and risk percentages.

## LLVM IR Generator
The code generator uses `LLVMContext`, `Module`, `IRBuilder`, functions, basic blocks, floating-point comparisons, conditional branches, and calls to `printf`. The generated `strategy_main` demonstrates signal execution using a small deterministic runtime model: price defaults to 95, while indicators default to 100. This keeps the compiler project independent of market data.

## Optimizer
LLVM's new pass manager runs its standard O2 per-module pipeline. The project does not implement optimization passes from scratch.

## Target generation
The optional object emitter initializes LLVM's native target and uses `TargetMachine` to emit a native object file.
