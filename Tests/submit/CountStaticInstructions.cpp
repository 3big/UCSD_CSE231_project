#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;
using namespace std;

namespace {
struct CSI : public FunctionPass {
  static char ID;
  CSI() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
  	std::map<const char*, int> inst_List;
    for(inst_iterator i = inst_begin(F), end = inst_end(F); i!=end; i++) {	
    	const char* inst_Name = i->getOpcodeName();
    	if(!inst_List.count(inst_Name)) {
    		inst_List[inst_Name] = 0;
    	}
    	inst_List[inst_Name]++;
    }
    for(auto inst: inst_List) {
    	errs()<< inst.first << "\t" << inst.second << "\n";
    }
    return false;
  }
}; // end of struct CSI
}  // end of anonymous namespace

char CSI::ID = 0;
static RegisterPass<CSI> X("cse231-csi", "Count Static Instructions Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
