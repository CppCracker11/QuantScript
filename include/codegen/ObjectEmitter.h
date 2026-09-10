#pragma once
#include <string>
namespace llvm { class Module; }
class ObjectEmitter {
public:
    bool emitObject(llvm::Module& module, const std::string& path, std::string& error) const;
};
