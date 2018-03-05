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
#include <string>


using namespace llvm;
using namespace std;

namespace llvm {
	class MayPointToInfo: public Info {
	public:
		MayPointToInfo() {}
		~MayPointToInfo() {}

		void print() {
			for(auto info : info_list) {
				errs()<< info.first << "->(" ;
				for(auto pointee : info.second) {
					errs()<< pointee << "/" ;
				}
				errs()<< ")|" ;	
			}
			errs()<< "\n";	
		}

		static bool equals(MayPointToInfo * info1, MayPointToInfo * info2) {
			if(info1->info_list.size()!= info2->info_list.size()) return false;
			for(auto in: info1->info_list) {
				if(info2->info_list[in.first] != in.second)
					return false;
			}
			return true;
		}

		static void join(MayPointToInfo * info1, MayPointToInfo * info2, MayPointToInfo * result) {
			for(auto in : info1->info_list) {		
				result->info_list[in.first].insert(in.second.begin(), in.second.end());	
			}
			for(auto in : info2->info_list) {		
				result->info_list[in.first].insert(in.second.begin(), in.second.end());	
			}
			return;
		}

		std::map<string, set<string> > info_list;
	};


	class MayPointToAnalysis : public DataFlowAnalysis<MayPointToInfo, true> { 
	public:
		MayPointToAnalysis(MayPointToInfo &InitialStates, MayPointToInfo &Bottom):
								   DataFlowAnalysis<MayPointToInfo, true>(InitialStates, Bottom) {}

		~MayPointToAnalysis() {}

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
															std::vector<MayPointToInfo *> & Infos)
		{
			unsigned index = InstrToIndex[I];
			MayPointToInfo *info_in = new MayPointToInfo();
			for(unsigned src : IncomingEdges) {
				Edge in_edge = std::make_pair(src, index);
				MayPointToInfo * edge_info = EdgeToInfo[in_edge];
				MayPointToInfo::join(info_in, edge_info, info_in);
			}

			unsigned op = (*I).getOpcode();
			MayPointToInfo* info_index = new MayPointToInfo();
			string index_str = std::to_string(index);

			if(op == 29) {
				info_index->info_list["R"+index_str].insert("M"+index_str);
			}
			
			if(op == 47) {
				Value* v = (*I).getOperand(0);
				Instruction* v_instr = dyn_cast<Instruction>(v);
				unsigned v_index = InstrToIndex[v_instr];
				string v_str = std::to_string(v_index);
				for(auto in: info_in->info_list) {
					if(in.first == "R"+v_str) {
						info_index->info_list["R"+index_str].insert(in.second.begin(), in.second.end());
					}
				}
			}
			
			if(op == 32) {
				Value* v = ((GetElementPtrInst*)I)->getPointerOperand();
				Instruction* v_instr = dyn_cast<Instruction>(v);
				unsigned v_index = InstrToIndex[v_instr];
				string v_str = std::to_string(v_index);
				for(auto in: info_in->info_list) {
					if(in.first == "R"+v_str) {
						info_index->info_list["R"+index_str].insert(in.second.begin(), in.second.end());
					}
				}
			}
			
			if(op == 30) {
				Value* v = ((LoadInst*)I)->getPointerOperand();
				Instruction* v_instr = dyn_cast<Instruction>(v);
				unsigned v_index = InstrToIndex[v_instr];
				string v_str = std::to_string(v_index);
				for(auto in: info_in->info_list) {
					if(in.first == "R"+v_str) {
						for (auto x: in.second) {
							if(info_in->info_list.count(x)) {
								std::set<string> tmp = info_in->info_list[x];
								info_index->info_list["R"+index_str].insert(tmp.begin(), tmp.end());
							}
						}
					}
				}
			}

			if(op == 31) {
				Value* v = ((StoreInst*)I)->getValueOperand(); 
				Value* v_ptr = ((StoreInst*)I)->getPointerOperand();
				Instruction* Rv_instr = dyn_cast<Instruction>(v);
				Instruction* Rp_instr = dyn_cast<Instruction>(v_ptr);
				unsigned Rv_idx = InstrToIndex[Rv_instr];
				unsigned Rp_idx = InstrToIndex[Rp_instr];
				string Rv_str = "R" + std::to_string(Rv_idx);
				string Rp_str = "R" + std::to_string(Rp_idx);

				if(info_in->info_list.count(Rv_str) && info_in->info_list.count(Rp_str) ) {
					std::set<string> tmp_Rv = info_in->info_list[Rv_str];
					std::set<string> tmp_Rp = info_in->info_list[Rp_str];

					for (auto y: tmp_Rp) {
						for(auto x: tmp_Rv) {
							info_index->info_list[y].insert(x);
						}
					}
				}

			}

			if (op == 55) {
				Value* v1 = ((SelectInst*)I)->getTrueValue(); 
				Value* v2 = ((SelectInst*)I)->getFalseValue();
				Instruction* Rv_instr = dyn_cast<Instruction>(v);
				Instruction* Rp_instr = dyn_cast<Instruction>(v_ptr);
				unsigned Rv_idx = InstrToIndex[Rv_instr];
				unsigned Rp_idx = InstrToIndex[Rp_instr];
				string Rv_str = "R" + std::to_string(Rv_idx);
				string Rp_str = "R" + std::to_string(Rp_idx);

			}

			if(op == 53) {

			}

			for(unsigned dst : OutgoingEdges) {
	  			Edge out_edge = std::make_pair(index, dst);
	  			MayPointToInfo* tmp_info = new MayPointToInfo();
				MayPointToInfo::join(info_in, info_index, tmp_info);
				Infos.push_back(tmp_info);
			}

			return;
		}

	};


	struct MayPointToAnalysisPass : public FunctionPass {
	  static char ID;
	  MayPointToAnalysisPass() : FunctionPass(ID) {}

	  bool runOnFunction(Function &F) override {
	  	MayPointToInfo InitialStates;
	  	MayPointToInfo Bottom;
	  	MayPointToAnalysis MPTA(InitialStates, Bottom);

	  	MPTA.runWorklistAlgorithm(&F);
	  	MPTA.print();
	  	
	    return false;
	  }
	}; // end of struct MayPointToAnalysisPass
}

char MayPointToAnalysisPass::ID = 0;
static RegisterPass<MayPointToAnalysisPass> X("cse231-maypointto", "May Point to Analysis Pass",
                             						  false /* Only looks at CFG */,
                             						  false /* Analysis Pass */);
