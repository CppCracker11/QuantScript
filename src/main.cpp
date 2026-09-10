#include "codegen/CodeGen.h"
#include "codegen/ObjectEmitter.h"
#include "lexer/Lexer.h"
#include "optimizer/Optimizer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <fstream>
#include <iostream>
#include <sstream>

static void help() {
    std::cout << "QuantScript compiler\nUsage: quantscript <file.qs> [options]\n\nOptions:\n  --tokens         print lexical tokens\n  --ast            print AST\n  --ir             print LLVM IR\n  --optimized-ir   print optimized LLVM IR\n  --emit-object    emit native object file\n  -o <file>        output file for IR/object\n  -h, --help       show this help\n";
}
static std::string readFile(const std::string& path) { std::ifstream in(path); if(!in) throw std::runtime_error("cannot open input file '"+path+"'"); std::stringstream ss; ss<<in.rdbuf(); return ss.str(); }
static void printTokens(const std::vector<Token>& ts) { for(const auto& t:ts) { std::cout<<tokenTypeName(t.type); if(!t.lexeme.empty()&&t.type!=TokenType::Newline) std::cout<<"("<<t.lexeme<<")"; std::cout<<" @ "<<t.location.line<<":"<<t.location.column<<"\n"; } }
static bool hasArg(int argc,char**argv,const std::string& a){for(int i=1;i<argc;i++)if(argv[i]==a)return true;return false;}
int main(int argc,char**argv){
    if(argc<2||hasArg(argc,argv,"-h")||hasArg(argc,argv,"--help")){help();return argc<2?1:0;}
    std::string input=argv[1], output; for(int i=2;i<argc;i++) if(std::string(argv[i])=="-o"){if(i+1>=argc){std::cerr<<"Error: -o requires a path\n";return 1;} output=argv[++i];}
    try {
        std::cout<<"Loading "<<input<<"\n";
        auto source=readFile(input);
        Lexer lexer(source); auto tokens=lexer.tokenize(); std::cout<<"[1/6] Lexical Analysis ........ OK\n"; if(hasArg(argc,argv,"--tokens")) printTokens(tokens);
        Parser parser(tokens); auto program=parser.parse(); std::cout<<"[2/6] Syntax Analysis ........ OK\n"; std::cout<<"[3/6] AST Generation ......... OK\n"; if(hasArg(argc,argv,"--ast")) program->print(std::cout);
        SemanticAnalyzer sem; sem.analyze(*program); std::cout<<"[4/6] Semantic Analysis ...... OK\n";
        CodeGenerator cg; auto module=cg.generate(*program); std::cout<<"[5/6] LLVM IR Generation ..... OK\n";
        if(hasArg(argc,argv,"--ir")) { if(output.empty()) module->print(llvm::outs(),nullptr); else { std::error_code ec; llvm::raw_fd_ostream out(output,ec); if(ec) throw std::runtime_error("cannot write IR: "+ec.message()); module->print(out,nullptr); } }
        Optimizer opt; opt.optimize(*module); if(llvm::verifyModule(*module,&llvm::errs())) throw std::runtime_error("optimized LLVM module verification failed"); std::cout<<"[6/6] Optimization ........... OK\n";
        if(hasArg(argc,argv,"--optimized-ir")) module->print(llvm::outs(),nullptr);
        if(hasArg(argc,argv,"--emit-object") || hasArg(argc,argv,"--compile")){ std::string path=output.empty()?"strategy.o":output; std::string err; ObjectEmitter e; if(!e.emitObject(*module,path,err)) throw std::runtime_error("object emission failed: "+err); std::cout<<"Object file: "<<path<<"\n"; }
        std::cout<<"Compilation successful.\n"; return 0;
    } catch(const CompilerError& e){ std::cerr<<e.what()<<"\n"; return 2; }
      catch(const std::exception& e){ std::cerr<<"Compiler Error: "<<e.what()<<"\n"; return 3; }
}
