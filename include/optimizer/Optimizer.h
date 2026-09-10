#pragma once
#include <string>
namespace llvm { class Module; }
class Optimizer {
public:
    void optimize(llvm::Module& module) const;
};
