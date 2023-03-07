//===-- Analyzer.cc - the kernel-analysis framework--------------===//
//
// This file implements the analysis framework. It calls the pass for
// building call-graph and the pass for finding security checks.
//
// ===-----------------------------------------------------------===//

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/SystemUtils.h>
#include <llvm/Support/ToolOutputFile.h>

#include <memory>
#include <sstream>
#include <sys/resource.h>
#include <vector>

#include "Analyzer.h"
#include "CallGraph.h"
#include "Config.h"
#include "HelperAnalysis.h"
#include "SecurityChecks.h"
#include "StackUsage.h"

using namespace llvm;

// Command line parameters.
cl::list<string> InputFilenames(cl::Positional, cl::OneOrMore,
                                cl::desc("<input bitcode files>"));

cl::opt<unsigned>
    VerboseLevel("verbose-level",
                 cl::desc("Print information at which verbose level"),
                 cl::init(0));

cl::opt<bool> SecurityChecks("sc", cl::desc("Identify sanity checks"),
                             cl::NotHidden, cl::init(false));

cl::list<string> CallgraphEntry("callgraph-entry",
                                cl::desc("Entry function of callgraphs"),
                                cl::OneOrMore, cl::CommaSeparated);

GlobalContext GlobalCtx;

void IterativeModulePass::run(ModuleList &modules) {

  ModuleList::iterator i, e;
  OP << "[" << ID << "] Initializing " << modules.size() << " modules ";
  bool again = true;
  while (again) {
    again = false;
    for (i = modules.begin(), e = modules.end(); i != e; ++i) {
      again |= doInitialization(i->first);
      OP << ".";
    }
  }
  OP << "\n";

  unsigned iter = 0, changed = 1;
  while (changed) {
    ++iter;
    changed = 0;
    unsigned counter_modules = 0;
    unsigned total_modules = modules.size();
    for (i = modules.begin(), e = modules.end(); i != e; ++i) {
      OP << "[" << ID << " / " << iter << "] ";
      OP << "[" << ++counter_modules << " / " << total_modules << "] ";
      OP << "[" << i->second << "]\n";

      bool ret = doModulePass(i->first);
      if (ret) {
        ++changed;
        OP << "\t [CHANGED]\n";
      } else
        OP << "\n";
    }
    OP << "[" << ID << "] Updated in " << changed << " modules.\n";
  }

  OP << "[" << ID << "] Postprocessing ...\n";
  again = true;
  while (again) {
    again = false;
    for (i = modules.begin(), e = modules.end(); i != e; ++i) {
      // TODO: Dump the results.
      again |= doFinalization(i->first);
    }
  }

  OP << "[" << ID << "] Done!\n\n";
}

void LoadStaticData(GlobalContext *GCtx) {

  // Load error-handling functions
  SetErrorHandleFuncs(GCtx->ErrorHandleFuncs);
  // load functions that copy/move values
  SetCopyFuncs(GCtx->CopyFuncs);
}

void ProcessResults(GlobalContext *GCtx) {}

void PrintResults(GlobalContext *GCtx) {

  OP << "############## Result Statistics ##############\n";
  OP << "# Number of sanity checks: \t\t\t" << GCtx->NumSecurityChecks << "\n";
  OP << "# Number of conditional statements: \t\t" << GCtx->NumCondStatements
     << "\n";
}

/// Migrate the data from constexpr array into global context
void initStackUsage(GlobalContext *GCtx) {
  auto IE = GCtx->AllFuncs.end();
  for (const auto &I : StackUsageArr) {
    // Somehow the kernel is mangling some of its symbols, i.e. functions
    // marked as `__init`, this means some of the functions in the stack
    // usage array will not be found in the global context
    auto IT = GCtx->AllFuncs.find(I.first);
    if (IT != IE)
      GCtx->StackUsage.insert({IT->second, I.second});
  }
}

int main(int argc, char **argv) {

  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);

  llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.

  cl::ParseCommandLineOptions(argc, argv, "global analysis\n");
  SMDiagnostic Err;

  // Loading modules
  OP << "Total " << InputFilenames.size() << " file(s)\n";

  for (unsigned i = 0; i < InputFilenames.size(); ++i) {

    LLVMContext *LLVMCtx = new LLVMContext();
    unique_ptr<Module> M = parseIRFile(InputFilenames[i], Err, *LLVMCtx);

    if (M == NULL) {
      OP << argv[0] << ": error loading file '" << InputFilenames[i] << "'\n";
      continue;
    }

    Module *Module = M.release();
    StringRef MName = StringRef(strdup(InputFilenames[i].data()));
    GlobalCtx.Modules.push_back(make_pair(Module, MName));
    GlobalCtx.ModuleMaps[Module] = InputFilenames[i];
  }

  // Main workflow
  LoadStaticData(&GlobalCtx);

  // Build global callgraph.
  CallGraphPass CGPass(&GlobalCtx);
  CGPass.run(GlobalCtx.Modules);

  OP << "Total " << CallgraphEntry.size() << " helpers\n";
  OP << "Total " << StackUsageArr.size()
     << " functions with stack depth data\n";
  OP << "Total " << GlobalCtx.AllFuncs.size() << " functions from IR\n";
  OP << "Total " << GlobalCtx.GlobalFuncs.size() << " global functions\n";

  initStackUsage(&GlobalCtx);

  OP << '\n';

  HelperAnalysisPass HAPass(&GlobalCtx);
  for (const string &helper : CallgraphEntry) {
    HAPass.treeWalk(GlobalCtx.GlobalFuncs[helper]);
  }

  OP << '\n';

  // Identify sanity checks
  if (SecurityChecks) {
    SecurityChecksPass SCPass(&GlobalCtx);
    SCPass.run(GlobalCtx.Modules);
  }

  // Print final results
  PrintResults(&GlobalCtx);

  return 0;
}
