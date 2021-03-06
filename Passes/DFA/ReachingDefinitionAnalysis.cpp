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
	class ReachingInfo: public Info {
	public:
		ReachingInfo() {}
		~ReachingInfo() {}

		void print() {
			for(auto info : info_list) {
				errs()<< info << "|" ;
			}
			errs()<< "\n";	
		}

		static bool equals(ReachingInfo * info1, ReachingInfo * info2) {
			return info1->info_list == info2->info_list;
		}

		static void join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * result) {
			result->info_list.insert(info1->info_list.begin(), info1->info_list.end());
			result->info_list.insert(info2->info_list.begin(), info2->info_list.end());
			return;
		}

		std::set<unsigned> info_list;
	};


	class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> { 
	public:
		ReachingDefinitionAnalysis(ReachingInfo &InitialStates, ReachingInfo &Bottom):
								   DataFlowAnalysis<ReachingInfo, true>(InitialStates, Bottom) {}

		~ReachingDefinitionAnalysis() {}

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
															std::vector<ReachingInfo *> & Infos)
		{
			unsigned index = InstrToIndex[I];
			ReachingInfo *info_in = new ReachingInfo();
			for(unsigned src : IncomingEdges) {
				Edge in_edge = std::make_pair(src, index);
				ReachingInfo * edge_info = EdgeToInfo[in_edge];
				ReachingInfo::join(info_in, edge_info, info_in);
			}

			ReachingInfo* info_index = new ReachingInfo();
			unsigned op = (*I).getOpcode();
			
			if(isa<BinaryOperator>(I) || op==29 || op==30 ||
			   op==32 || op==51 || op==52 || op==55) 
			{
				info_index->info_list.insert(index);
			}
			if(op == 53) {
				BasicBlock* block = I->getParent();
          		for (auto ii = block->begin(), ie = block->end(); ii != ie; ++ii) {
            		Instruction * instr = &*ii;
            		if (isa<PHINode>(instr)){
              			info_index->info_list.insert(InstrToIndex[instr]);
            		}
          		}
			}

			for(unsigned dst : OutgoingEdges) {
	  			Edge out_edge = std::make_pair(index, dst);
	  			ReachingInfo* tmp_info = new ReachingInfo();
				ReachingInfo::join(info_in, info_index, tmp_info);
				Infos.push_back(tmp_info);
			}

			return;
		}

	};


	struct ReachingDefinitionAnalysisPass : public FunctionPass {
	  static char ID;
	  ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

	  bool runOnFunction(Function &F) override {
	  	ReachingInfo InitialStates;
	  	ReachingInfo Bottom;
	  	ReachingDefinitionAnalysis RDA(InitialStates, Bottom);

	  	RDA.runWorklistAlgorithm(&F);
	  	RDA.print();
	  	
	    return false;
	  }
	}; // end of struct ReachingDefinitionAnalysisPass
}

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "Count Reaching Definition Pass",
                             						  false /* Only looks at CFG */,
                             						  false /* Analysis Pass */);
