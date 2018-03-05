#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include <cstdint>
#include "231DFA.h"
#include <vector>
#include <map>
#include <set>


using namespace llvm;
using namespace std;

namespace llvm {
	class LivenessInfo: public Info {
	public:
		LivenessInfo() {}
		~LivenessInfo() {}

		void print() {
			for(auto info : info_list) {
				errs()<< info << "|" ;
			}
			errs()<< "\n";	
		}

		static bool equals(LivenessInfo * info1, LivenessInfo * info2) {
			return info1->info_list == info2->info_list;
		}

		static void join(LivenessInfo * info1, LivenessInfo * info2, LivenessInfo * result) {
			result->info_list.insert(info1->info_list.begin(), info1->info_list.end());
			result->info_list.insert(info2->info_list.begin(), info2->info_list.end());
			return;
		}

		std::set<unsigned> info_list;
	};


	class LivenessAnalysis : public DataFlowAnalysis<LivenessInfo, false> { 
	public:
		LivenessAnalysis(LivenessInfo &InitialStates, LivenessInfo &Bottom):
								   DataFlowAnalysis<LivenessInfo, false>(InitialStates, Bottom) {}

		~LivenessAnalysis() {}

	    /*
     	* The flow function.
     	*   Instruction I: the IR instruction to be processed.
     	*   std::vector<unsigned> & IncomingEdges: the vector of the indices of the source instructions of the incoming edges.
     	*   std::vector<unsigned> & OutgoingEdges: the vector of indices of the source instructions of the outgoing edges.
     	*   std::vector<Info *> & Infos: the vector of the newly computed information for each outgoing eages.
     	*
     	* Direction:
     	* 	 Implement this function in subclasses.
     	*/
		void flowfunction(Instruction * I,
    													std::vector<unsigned> & IncomingEdges,
															std::vector<unsigned> & OutgoingEdges,
															std::vector<LivenessInfo *> & Infos)
		{
			unsigned index = InstrToIndex[I];
			Infos.resize(OutgoingEdges.size());

			// join all infos from incoming edges
			LivenessInfo *info_in = new LivenessInfo();

			for(unsigned src : IncomingEdges) {
				Edge in_edge = std::make_pair(src, index);
				LivenessInfo * edge_info = EdgeToInfo[in_edge];
				LivenessInfo::join(info_in, edge_info, info_in);
			}

			unsigned op = (*I).getOpcode();

			if(op == 53) {
				BasicBlock* block = I->getParent();

				for (auto ii = block->begin(), ie = block->end(); ii != ie; ++ii) {
            		Instruction * instr = &*ii;
            		if (isa<PHINode>(instr)) {
						info_in->info_list.erase(InstrToIndex[instr]);
					}
				}
				for (unsigned j=0; j<OutgoingEdges.size(); j++) {
	            	Infos[j] = new LivenessInfo();
	            	Infos[j]->info_list = info_in->info_list;
	            }
          		for (auto ii = block->begin(), ie = block->end(); ii != ie; ++ii) {
            		Instruction * instr = &*ii;
            		if (isa<PHINode>(instr)){
            			LivenessInfo* info_out = new LivenessInfo();
            			LivenessInfo::join(info_in, info_out, info_out);
            			PHINode* phiInstr = (PHINode*) instr;
            			
	          			for(unsigned i = 0; i < phiInstr->getNumIncomingValues(); i++) {
	            			Instruction* phiValue = (Instruction*)(phiInstr->getIncomingValue(i));
	            			if(InstrToIndex.find((Instruction*) phiValue) == InstrToIndex.end()) continue;
	                		unsigned value_src = InstrToIndex[phiValue];
	                		BasicBlock* label_block = phiInstr->getIncomingBlock(i);
	                		Instruction* label_instr = (Instruction *)label_block->getTerminator();
	                		unsigned label = InstrToIndex[label_instr];
							for (unsigned j=0; j<OutgoingEdges.size(); j++) { 	
								unsigned dst = OutgoingEdges[j];      	
	            				if(dst == label) {
									Infos[j]->info_list.insert(value_src);	
	            				}
	            			}

	            		}
            		}

          		}
          		return;
			}

			LivenessInfo* info_operand = new LivenessInfo();
			for(unsigned i=0; i<(*I).getNumOperands(); i++) {
				Value* v= (*I).getOperand(i);
				Instruction* operand = dyn_cast<Instruction>(v);
				unsigned operand_index = InstrToIndex[operand];
				if(operand_index!=0) {
					info_operand->info_list.insert(operand_index);	
				}
			}

			LivenessInfo* info_out = new LivenessInfo();

			LivenessInfo::join(info_in, info_operand, info_out);
			
			
			if(isa<BinaryOperator>(I) || op==29 || op==30 ||
			   op==32 || op==51 || op==52 || op==55) 
			{
				info_out->info_list.erase(index);
			}
			
			for (unsigned i=0; i<Infos.size(); i++) {
				Infos[i] = info_out;
			}
			
			return;
		}


	// private:
	// 	std::map<const char*, int> varList;
	};


	struct LivenessAnalysisPass : public FunctionPass {
	  static char ID;
	  LivenessAnalysisPass() : FunctionPass(ID) {}

	  bool runOnFunction(Function &F) override {
	  	LivenessInfo InitialStates;
	  	LivenessInfo Bottom;
	  	LivenessAnalysis LA(InitialStates, Bottom);

	  	LA.runWorklistAlgorithm(&F);
	  	LA.print();
	  	
	    return false;
	  }
	}; // end of struct LivenessAnalysisPass
}

char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> X("cse231-liveness", "Liveness Analysis Pass",
                             						  false /* Only looks at CFG */,
                             						  false /* Analysis Pass */);
