// ECE/CS 5544 S22 Assignment 2: reaching.cpp
// Group:

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;

namespace {

  class Reaching : public FunctionPass {
  public:
    static char ID;

    Reaching() : FunctionPass(ID) { }

    virtual bool runOnFunction(Function& F) {

      // Did not modify the incoming Function.
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:
  };

  char Reaching::ID = 0;
  RegisterPass<Reaching> X("reaching", "ECE/CS 5544 Reaching");
}
