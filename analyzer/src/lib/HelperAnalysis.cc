#include "HelperAnalysis.h"

#include "Analyzer.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>

#include <queue>
#include <system_error>

using namespace llvm;

/// Use a worklist algorithm to perform a tree walk over the callgraph
void HelperAnalysisPass::treeWalk(Function *HelperFn) {
  std::queue<Function *> WorkList;
  WorkList.push(HelperFn);

  while (!WorkList.empty()) {
    Function *CurrFn = WorkList.front();
    WorkList.pop();

    // Avoid visiting the same function twice
    if (AdjList.count(CurrFn))
      continue;

    // Insertion happens implicitly here
    SmallPtrSetImpl<Function *> &Neighbors = AdjList[CurrFn];

    // Traverse all call sites in current function
    for (inst_iterator I = inst_begin(CurrFn), E = inst_end(CurrFn); I != E;
         ++I) {
      if (auto *CI = dyn_cast<CallInst>(&*I)) {
        for (Function *Callee : Ctx->Callees[CI]) {
          Neighbors.insert(Callee);
          WorkList.push(Callee);
        }
      }
    }
  }
}

/// Dump the callgragh in the adjacency list format
void HelperAnalysisPass::dumpCG(StringRef Filename) const {
  std::error_code EC;
  raw_fd_ostream OS(Filename, EC);
  if (EC)
    return;

  // Dump the AdjList
  for (const auto &It : AdjList) {
    const Function *CurrFn = It.first;
    OS << CurrFn->getName() << ' ';

    for (Function *const &AdjFn : It.second)
      OS << AdjFn->getName() << ' ';

    OS << '\n';
  }
}
