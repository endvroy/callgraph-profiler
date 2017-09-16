

#ifndef PROFILING_INSTRUMENTATION_PASS_H
#define PROFILING_INSTRUMENTATION_PASS_H

#include <vector>
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace cgprofiler {


struct ProfilingInstrumentationPass : public llvm::ModulePass {
  static char ID;
  llvm::DenseMap<llvm::Function*, uint64_t> fn_id_map;
  std::vector<llvm::Function*> all_fn;

  ProfilingInstrumentationPass() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module& m) override;
  void handleInstruction(llvm::Module& m,
                         llvm::CallSite cs,
                         llvm::Function*,
                         llvm::Value* fp_fn,
                         llvm::Value* counter);
};
}


#endif
