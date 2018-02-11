#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include <cstdint>


using namespace llvm;
using namespace std;

namespace {
struct BranchBias : public FunctionPass {
  static char ID;
  BranchBias() : FunctionPass(ID) {}


  virtual bool runOnFunction(Function &F) {
    Function* printBranch;
    Function* updateBranch;
    LLVMContext& Ctx = F.getContext();
    Module *M = F.getParent();
    Constant* printBranchFunc = M->getOrInsertFunction("printOutBranchInfo",
                                                      Type::getVoidTy(Ctx),
                                                      NULL);
    Constant* updateBranchFunc = M->getOrInsertFunction("updateBranchInfo",
                                                      Type::getVoidTy(Ctx),
                                                      IntegerType::get(Ctx,1));
    printBranch = cast<Function>(printBranchFunc);
    updateBranch = cast<Function>(updateBranchFunc);

    for(Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
      for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI) {
        if((*BI).getOpcode() == 2 && (*BI).getNumOperands() > 1) {
            Value* taken = (*BI).getOperand(0);
            vector<Value*> args;
            args.push_back(taken);
            Instruction* ip = &*BI;
            IRBuilder<> builder(ip);
            builder.CreateCall(updateBranch, args);
        }
        if((*BI).getOpcode() == 1) {
            Instruction* ip = &*BI;
            IRBuilder<> builder(ip);
            builder.CreateCall(printBranch);
        }
      }
    }
    return false;
  }

}; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profile Branch Bias Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
