// ECE/CS 5544 S22 Assignment 2: available.cpp
// Group:

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "available-support.h"

using namespace llvm;
using namespace std;

namespace {

  class Analysis : public Dataflow {
      public:
        Analysis (Direction direction, MeetOp meetop, Domain domain)
            : Dataflow (direction, meetop, domain) {}
        
        TransferOutput transferFn (VSet input, BasicBlock *curr) {
            TransferOutput output;
            VSet gen, kill;
            for (auto inst = curr->begin(); inst != curr->end(); ++inst) {
                std::string val;
                if (isa<Instruction> (&*inst)) {
                    val = getShortValueName(&*inst);
                }

                if (auto load_ins = dyn_cast<StoreInst>(&*inst)) {
                    val = getShortValueName(load_ins->getPointerOperand());
                }
                
                // skip if it's not a value
                if (val == "") {
                    continue;
                }

                // if input has be re-assigned, kill it
                for (auto LHS : input) {
                    Expression* exp = (Expression *) domain[LHS];
                    std::string left = getShortValueName(exp->v1), right = getShortValueName(exp->v2);
                    if (left == val || right == val) {
                        kill.insert(LHS);
                    }    
                }

                // some values are killed in the same Basic Block, so sad
                auto tmp = gen.end();
                for (auto i = gen.begin(); i != gen.end(); ++i) {
                    if (tmp != gen.end()) {
                        gen.erase(tmp);
                        tmp = gen.end();
                    }
                    Expression* exp = (Expression *) domain[*i];
                    std::string left = getShortValueName(exp->v1), right = getShortValueName(exp->v2);                        
                    if (left == val || right == val) {
                        tmp = i;
                    }
                }

                // insert to generate set
                if (isa<BinaryOperator> (&*inst)) {
                    Expression exp = Expression(&*inst);
                    int i = 0;
                    for (auto e : domain) {
                        if (exp == *(Expression *)(e)) {
                            break;
                        }
                        i++;
                    }

                    if (i < domain.size()) {
                        gen.insert(i);
                    }
                }
            }

            auto tmp = substractSet(input, kill);
            output.transfer = unionSet(tmp, gen);
            return output;
        }
  };

  class AvailableExpressions : public FunctionPass {
    
    public:
      static char ID;
      
      AvailableExpressions() : FunctionPass(ID) { }
      
      virtual bool runOnFunction(Function& F) {

            outs() << "Function: " << F.getName() << "\n";
            DataFlowResult result;
            Domain domain;

            // we only evaluate Expressions
            for (auto I = inst_begin(F); I != inst_end(F); ++I) {
                if (isa<BinaryOperator> (*I)) {
                    domain.push_back(new Expression(&*I));
                }
            }

            Analysis analysis = Analysis(Direction::FORWARD, MeetOp::INTERSECT,  domain);
            result = analysis.run(F, VSet(), VSet());

            // output the result
            int index = 0;
            for (auto &BB : F) {
                outs() << "\n<" << BB.getName() << ">\n";
                VSet gen = result.result[&BB].in;
                for (auto &I : BB) {
                    outs() << index << ": " << I;

                    std::string val = getShortValueName(&I);
                    
                    // killed redefined expressions
                    auto tmp = gen.end();
                    for (auto i = gen.begin(); i != gen.end(); ++i) {
                        if (tmp != gen.end()) {
                            gen.erase(tmp);
                            tmp = gen.end();
                        }
                        Expression* exp = (Expression *) domain[*i];
                        std::string left = getShortValueName(exp->v1), right = getShortValueName(exp->v2);                        
                        if (left == val || right == val) {
                            tmp = i;
                        }
                    }
                    
                    // insert expressions
                    if (isa<BinaryOperator> (&I)) {
                        Expression exp = Expression(&I);
                        int i = 0;
                        for (auto e : domain) {
                            if (exp == *(Expression *)(e)) {
                                break;
                            }
                            i++;
                        }
                        gen.insert(i);
                    }
                    
                    // pretty print
                    outs() << "\t{";
                    for (auto i : gen) {
                        outs() << ((Expression *) domain[i])->toString() << ", ";
                    }
                    outs() << "}\n";

                    index++;
                }    
            }
            // clean up
            for (auto exp : domain) {
                delete (Expression *) exp;
            }

            return false;
      }
      
      virtual void getAnalysisUsage(AnalysisUsage& AU) const {
        AU.setPreservesAll();
        
        // Here's some code to familarize you with the Expression
        // class and pretty printing code we've provided:
        
        /*vector<Expression> expressions;
        for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
           BasicBlock* block = &*FI;
           for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
              
             Instruction* I = &*i;
             // We only care about available expressions for BinaryOperators
             if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
               
               expressions.push_back(Expression(BI));
             }
           }
        }
        
        // Print out the expressions used in the function
        outs() << "Expressions used by this function:\n";
        printSet(&expressions);
        
        // Did not modify the incoming Function.
        return false;*/
      }
      
    private:

  };
  
  char AvailableExpressions::ID = 0;
  RegisterPass<AvailableExpressions> X("available",
				       "ECE/CS 5544 Available Expressions");
}
