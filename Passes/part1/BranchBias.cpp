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

    // for(Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
    //     runOnBasicBlock(BB, F);
    // }

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

  // virtual bool runOnBasicBlock(Function::iterator &BB, Function &F) {
  //   uint32_t keys[64]={0};
  //   uint32_t values[64]={0};
  //   std::map<uint32_t, uint32_t> hash;

  //   Function* updteInstr;
  //   LLVMContext& Ctx = F.getContext();
  //   Module *M = F.getParent();
  //   Constant* updateInfo = M->getOrInsertFunction("updateInstrInfo", 
  //                                                 FunctionType::getVoidTy(M->getContext()), 
  //                                                 Type::getInt32Ty(M->getContext()), 
  //                                                 Type::getInt32PtrTy(M->getContext()), 
  //                                                 Type::getInt32PtrTy(M->getContext()));
  //   updteInstr = cast<Function>(updateInfo);

  //   for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI) {
  //     uint32_t code= (*BI).getOpcode();
  //     if(hash.count(code)) {
  //       values[hash[code]]++;
  //     }
  //     else {
  //       int count = hash.size();
  //       keys[count] = code;
  //       values[count]=0;
  //       values[count]++;
  //       hash[code] = count;
  //     }
  //   }
  //   BasicBlock::iterator end = BB->end();
  //   Instruction* ip = &*(--end);
  //   IRBuilder<> builder(ip);
  //   ArrayType* ArrayTy = ArrayType::get(IntegerType::get(F.getContext(),32), 64);
  //   GlobalVariable* arg1 = new GlobalVariable(*(F.getParent()), 
  //                                             // Type::getInt32Ty(M->getContext()),
  //                                             ArrayTy,
  //                                             true,
  //                                             GlobalValue::InternalLinkage,
  //                                             ConstantDataArray::get(Ctx, keys),
  //                                             "keys global");
  //   GlobalVariable* arg2 = new GlobalVariable(*(F.getParent()),
  //                                             // Type::getInt32Ty(M->getContext()),
  //                                             ArrayTy,
  //                                             true,
  //                                             GlobalValue::InternalLinkage,
  //                                             ConstantDataArray::get(Ctx, values),
  //                                             "values global");                
  //   std::vector<Value*> args;
  //   args.push_back(builder.getInt32(hash.size()));
  //   args.push_back(builder.CreatePointerCast(arg1, Type::getInt32PtrTy(Ctx)));
  //   args.push_back(builder.CreatePointerCast(arg2, Type::getInt32PtrTy(Ctx)));
  //   builder.CreateCall(updteInstr, args);
    
  //   return false;
  // }

}; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profile Branch Bias Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
