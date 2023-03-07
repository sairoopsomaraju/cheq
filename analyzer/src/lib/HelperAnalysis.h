#ifndef HELPER_ANALYSIS_H
#define HELPER_ANALYSIS_H

#include <llvm/IR/Function.h>

struct GlobalContext;

class HelperAnalysisPass {
  GlobalContext *Ctx;

public:
  explicit HelperAnalysisPass(GlobalContext *Ctx) : Ctx(Ctx) {}

  ~HelperAnalysisPass() = default;

  /// Prevent accidental copy/move of the pass
  HelperAnalysisPass(const HelperAnalysisPass &) = delete;
  HelperAnalysisPass(HelperAnalysisPass &&) = delete;
  HelperAnalysisPass &operator=(const HelperAnalysisPass &) = delete;
  HelperAnalysisPass &operator=(HelperAnalysisPass &&) = delete;

  void treeWalk(llvm::Function *);
};

#endif /* HELPER_ANALYSIS_H */
