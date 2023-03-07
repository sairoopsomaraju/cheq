#include "HelperAnalysis.h"

#include "Analyzer.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>

#include <queue>

using namespace llvm;

/// Use a worklist algorithm to perform a tree walk over the callgraph
void HelperAnalysisPass::treeWalk(Function *HelperFn) {
  DenseSet<Function *> Visited;
  std::queue<Function *> WorkList;

  WorkList.push(HelperFn);

  while (!WorkList.empty()) {
    Function *CurrFn = WorkList.front();
    WorkList.pop();

    // Avoid visiting the same function twice
    if (Visited.count(CurrFn))
      continue;
    Visited.insert(CurrFn);
    OP << CurrFn->getName() << " stack-usage=" << Ctx->StackUsage[CurrFn]
       << '\n';

    // Traverse all call sites in current function
    for (inst_iterator I = inst_begin(CurrFn), E = inst_end(CurrFn); I != E;
         ++I) {
      if (auto *CI = dyn_cast<CallInst>(&*I)) {
        for (Function *Callee : Ctx->Callees[CI]) {
          WorkList.push(Callee);
        }
      }
    }
  }
}
