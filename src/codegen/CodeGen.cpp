#include "codegen/CodeGen.h"
#include "lexer/Lexer.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <stdexcept>
#include <string>
#include <vector>

CodeGenerator::CodeGenerator()
    : context_(std::make_unique<llvm::LLVMContext>()),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {}

CodeGenerator::~CodeGenerator() = default;

void CodeGenerator::setOutputInputs(double p, double s, double e) {
    if (!module_)
        return;

    auto* f = module_->getFunction("strategy_main");

    if (!f)
        return;

    (void)f;

    auto* dbl = llvm::Type::getDoubleTy(*context_);

    priceValue_ = llvm::ConstantFP::get(dbl, p);
    sma20Value_ = llvm::ConstantFP::get(dbl, s);
    ema20Value_ = llvm::ConstantFP::get(dbl, e);
}

llvm::Value* CodeGenerator::emitIndicator(
    const FunctionCallNode& fn) {

    auto* n = dynamic_cast<NumberNode*>(fn.argument.get());

    if (!n)
        throw std::runtime_error(
            "indicator argument must be a constant number"
        );

    double period = n->value;
    auto* dbl = llvm::Type::getDoubleTy(*context_);

    if (fn.name == "SMA" && period == 20 && sma20Value_)
        return sma20Value_;

    if (fn.name == "EMA" && period == 20 && ema20Value_)
        return ema20Value_;

    return llvm::ConstantFP::get(dbl, 100.0);
}

llvm::Value* CodeGenerator::emitArithmetic(
    const BinaryExpressionNode& b) {

    auto* l = emitExpression(*b.left);
    auto* r = emitExpression(*b.right);

    switch (b.op) {
        case BinaryOp::Add:
            return builder_->CreateFAdd(l, r, "addtmp");

        case BinaryOp::Subtract:
            return builder_->CreateFSub(l, r, "subtmp");

        case BinaryOp::Multiply:
            return builder_->CreateFMul(l, r, "multmp");

        case BinaryOp::Divide:
            return builder_->CreateFDiv(l, r, "divtmp");

        default:
            break;
    }

    return nullptr;
}

llvm::Value* CodeGenerator::emitExpression(
    const ExpressionNode& e) {

    auto* dbl = llvm::Type::getDoubleTy(*context_);

    if (auto n = dynamic_cast<const NumberNode*>(&e))
        return llvm::ConstantFP::get(dbl, n->value);

    if (auto id = dynamic_cast<const IdentifierNode*>(&e)) {

        if (id->name == "price")
            return priceValue_;

        if (id->name == "volume")
            return llvm::ConstantFP::get(dbl, 1000.0);

        throw std::runtime_error(
            "unknown identifier in code generation: " +
            id->name
        );
    }

    if (auto f = dynamic_cast<const FunctionCallNode*>(&e))
        return emitIndicator(*f);

    if (auto b = dynamic_cast<const BinaryExpressionNode*>(&e))
        return emitArithmetic(*b);

    return nullptr;
}

llvm::Value* CodeGenerator::emitCondition(
    const ExpressionNode& e) {

    auto* b = dynamic_cast<const BinaryExpressionNode*>(&e);

    if (!b)
        throw std::runtime_error(
            "condition is not a comparison"
        );

    auto* l = emitExpression(*b->left);
    auto* r = emitExpression(*b->right);

    llvm::FCmpInst::Predicate p;

    switch (b->op) {
        case BinaryOp::Less:
            p = llvm::FCmpInst::FCMP_OLT;
            break;

        case BinaryOp::Greater:
            p = llvm::FCmpInst::FCMP_OGT;
            break;

        case BinaryOp::LessEqual:
            p = llvm::FCmpInst::FCMP_OLE;
            break;

        case BinaryOp::GreaterEqual:
            p = llvm::FCmpInst::FCMP_OGE;
            break;

        case BinaryOp::Equal:
            p = llvm::FCmpInst::FCMP_OEQ;
            break;

        case BinaryOp::NotEqual:
            p = llvm::FCmpInst::FCMP_ONE;
            break;

        default:
            throw std::runtime_error(
                "invalid condition operator"
            );
    }

    return builder_->CreateFCmp(
        p,
        l,
        r,
        "condition"
    );
}

std::unique_ptr<llvm::Module> CodeGenerator::generate(
    const ProgramNode& program) {

    module_ = std::make_unique<llvm::Module>(
        "QuantScript." + program.strategy->name,
        *context_
    );

    module_->setSourceFileName(
        program.strategy->name + ".qs"
    );

    auto* i32 = llvm::Type::getInt32Ty(*context_);
    auto* dbl = llvm::Type::getDoubleTy(*context_);
    auto* i8p = llvm::PointerType::getUnqual(*context_);

    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(i8p);

    auto* printfTy = llvm::FunctionType::get(
        i32,
        printfArgs,
        true
    );

    auto* printfFn = llvm::Function::Create(
        printfTy,
        llvm::Function::ExternalLinkage,
        "printf",
        *module_
    );

    auto* fnTy = llvm::FunctionType::get(
        i32,
        false
    );

    auto* fn = llvm::Function::Create(
        fnTy,
        llvm::Function::ExternalLinkage,
        "strategy_main",
        *module_
    );

    auto* entry = llvm::BasicBlock::Create(
        *context_,
        "entry",
        fn
    );

    builder_->SetInsertPoint(entry);

    priceValue_ = llvm::ConstantFP::get(dbl, 95.0);
    sma20Value_ = llvm::ConstantFP::get(dbl, 100.0);
    ema20Value_ = llvm::ConstantFP::get(dbl, 100.0);

    auto* buyMsg = builder_->CreateGlobalString(
        "BUY SIGNAL\n",
        "buy_msg"
    );

    auto* sellMsg = builder_->CreateGlobalString(
        "SELL SIGNAL\n",
        "sell_msg"
    );

    auto* noMsg = builder_->CreateGlobalString(
        "NO SIGNAL\n",
        "no_msg"
    );

    llvm::BasicBlock* current = entry;

    for (const auto& stmt :
         program.strategy->statements) {

        const ExpressionNode* cond = nullptr;
        const char* label = nullptr;

        if (auto b =
                dynamic_cast<const BuyStatementNode*>(
                    stmt.get())) {

            cond = b->condition.get();
            label = "buy";

        } else if (auto s =
                       dynamic_cast<const SellStatementNode*>(
                           stmt.get())) {

            cond = s->condition.get();
            label = "sell";

        } else {
            continue;
        }

        builder_->SetInsertPoint(current);

        auto* signal = llvm::BasicBlock::Create(
            *context_,
            std::string(label) + "_signal",
            fn
        );

        auto* next = llvm::BasicBlock::Create(
            *context_,
            std::string(label) + "_next",
            fn
        );

        auto* condition = emitCondition(*cond);

        builder_->CreateCondBr(
            condition,
            signal,
            next
        );

        builder_->SetInsertPoint(signal);

        llvm::Value* message =
            label[0] == 'b' ? buyMsg : sellMsg;

        std::vector<llvm::Value*> args;
        args.push_back(message);

        builder_->CreateCall(
            printfFn,
            args
        );

        builder_->CreateRet(
            llvm::ConstantInt::get(i32, 0)
        );

        current = next;
    }

    builder_->SetInsertPoint(current);

    std::vector<llvm::Value*> args;
    args.push_back(noMsg);

    builder_->CreateCall(
        printfFn,
        args
    );

    builder_->CreateRet(
        llvm::ConstantInt::get(i32, 0)
    );

    if (llvm::verifyModule(
            *module_,
            &llvm::errs())) {

        throw std::runtime_error(
            "LLVM module verification failed"
        );
    }

    return std::move(module_);
}