#pragma once
#include "ast/AST.h"
#include <memory>
#include <string>
#include <llvm/IR/IRBuilder.h>

namespace llvm { class LLVMContext; class Module; class Value; }

class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();
    std::unique_ptr<llvm::Module> generate(const ProgramNode& program);
    void setOutputInputs(double price, double sma20 = 100.0, double ema20 = 100.0);
private:
    llvm::Value* emitExpression(const ExpressionNode& expr);
    llvm::Value* emitCondition(const ExpressionNode& expr);
    llvm::Value* emitIndicator(const FunctionCallNode& fn);
    llvm::Value* emitArithmetic(const BinaryExpressionNode& bin);
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::Module> module_;
    llvm::Value* priceValue_ = nullptr;
    llvm::Value* sma20Value_ = nullptr;
    llvm::Value* ema20Value_ = nullptr;
};
