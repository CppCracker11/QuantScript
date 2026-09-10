# QuantScript Language Specification

## Keywords
`STRATEGY`, `BUY`, `SELL`, `IF`, `STOPLOSS`, `TAKEPROFIT`, `SMA`, `EMA`.

## Built-in identifiers
`price` and `volume` are predefined numeric inputs.

## Literals
Integers such as `20` and floating-point numbers such as `100.5` are supported. Percentages are written as a number followed by `%`.

## Operators
Arithmetic: `+ - * /`

Comparison: `< > <= >= == !=`

Parentheses may group expressions.

## Statements
A program contains exactly one strategy declaration followed by one or more trading/risk statements.

```text
STRATEGY MeanReversion
BUY IF price < SMA(20)
SELL IF price > SMA(20)
STOPLOSS 2%
TAKEPROFIT 5%
```

## Indicators
`SMA(n)` and `EMA(n)` are compiler-recognized built-ins. Their period must be a positive integer. The current LLVM demonstration runtime models indicator values with simple constants rather than implementing a historical price-series engine.

## Invalid examples
```text
STRATEGY Broken
BUY IF abc > SMA(20)       # abc is undefined
```

```text
STRATEGY Broken
BUY IF price < SMA(0)      # invalid period
```

```text
STRATEGY Broken
STOPLOSS -5%               # invalid risk percentage
```
