#include "optimizer/Optimizer.h"
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>

void Optimizer::optimize(llvm::Module& m) const {
    llvm::PassBuilder pb;
    llvm::LoopAnalysisManager lam; llvm::FunctionAnalysisManager fam; llvm::CGSCCAnalysisManager cgam; llvm::ModuleAnalysisManager mam;
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam); pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam); pb.crossRegisterProxies(lam,fam,cgam,mam);
    auto mpm=pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2); mpm.run(m,mam);
}
