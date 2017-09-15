

#ifndef PROFILING_INSTRUMENTATION_PASS_H
#define PROFILING_INSTRUMENTATION_PASS_H

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
  llvm::DenseMap<llvm::StringRef, uint64_t> name_id_map;

  ProfilingInstrumentationPass() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module& m) override;
  void handleInstruction(llvm::CallSite cs,
                         llvm::Function*,
                         llvm::Value* counter);
};
}


#endif
