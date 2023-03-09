#ifndef HELPER_ANALYSIS_H
#define HELPER_ANALYSIS_H

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringRef.h>

/// Forward declarations
struct GlobalContext;

namespace llvm {
class Function;
}

class HelperAnalysisPass {
  GlobalContext *Ctx;
  llvm::DenseMap<llvm::Function *, llvm::SmallPtrSet<llvm::Function *, 8>>
      AdjList;

public:
  explicit HelperAnalysisPass(GlobalContext *Ctx) : Ctx(Ctx) {}

  ~HelperAnalysisPass() = default;

  /// Prevent accidental copy/move of the pass
  HelperAnalysisPass(const HelperAnalysisPass &) = delete;
  HelperAnalysisPass(HelperAnalysisPass &&) = delete;
  HelperAnalysisPass &operator=(const HelperAnalysisPass &) = delete;
  HelperAnalysisPass &operator=(HelperAnalysisPass &&) = delete;

  void treeWalk(llvm::Function *);
  void dumpCG(llvm::StringRef) const;

  void clear() { AdjList.clear(); }
};

#endif /* HELPER_ANALYSIS_H */
