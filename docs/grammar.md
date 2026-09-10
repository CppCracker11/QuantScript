# QuantScript Grammar

The grammar is intentionally small and maps closely to the handwritten recursive-descent parser.

```text
program             -> NEWLINE* strategy NEWLINE* EOF
strategy            -> STRATEGY IDENTIFIER NEWLINE* statement*
statement           -> buyStatement | sellStatement | stopLossStatement | takeProfitStatement
buyStatement        -> BUY IF expression
sellStatement       -> SELL IF expression
stopLossStatement   -> STOPLOSS NUMBER PERCENTAGE
takeProfitStatement -> TAKEPROFIT NUMBER PERCENTAGE
expression          -> comparison
comparison          -> arithmeticExpression (comparisonOperator arithmeticExpression)?
arithmeticExpression-> term ((PLUS | MINUS) term)*
term                -> factor ((STAR | SLASH) factor)*
factor              -> NUMBER | IDENTIFIER | functionCall | MINUS factor | '(' expression ')'
functionCall        -> (SMA | EMA) '(' expression ')'
comparisonOperator  -> '<' | '>' | '<=' | '>=' | '==' | '!='
NUMBER              -> INTEGER | FLOAT
```

Semantic analysis further restricts indicator periods to positive integers and BUY/SELL conditions to boolean comparison expressions.
