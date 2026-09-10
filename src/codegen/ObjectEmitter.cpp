#include "codegen/ObjectEmitter.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <memory>
#include <optional>
#include <string>

bool ObjectEmitter::emitObject(
    llvm::Module& module,
    const std::string& path,
    std::string& error) const {

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    module.setTargetTriple(triple);

    std::string err;
    const llvm::Target* target =
        llvm::TargetRegistry::lookupTarget(triple, err);

    if (!target) {
        error = err;
        return false;
    }

    llvm::TargetOptions options;

    auto targetMachine = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(
            triple,
            "generic",
            "",
            options,
            std::nullopt
        )
    );

    if (!targetMachine) {
        error = "could not create target machine";
        return false;
    }

    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;

    llvm::raw_fd_ostream output(
        path,
        ec,
        llvm::sys::fs::OF_None
    );

    if (ec) {
        error = ec.message();
        return false;
    }

    llvm::legacy::PassManager passManager;

    if (targetMachine->addPassesToEmitFile(
            passManager,
            output,
            nullptr,
            llvm::CodeGenFileType::ObjectFile)) {

        error = "target cannot emit object files";
        return false;
    }

    passManager.run(module);
    output.flush();

    return true;
}