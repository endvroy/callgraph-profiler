

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <iostream>

#include "ProfilingInstrumentationPass.h"

using namespace llvm;
using cgprofiler::ProfilingInstrumentationPass;


namespace cgprofiler {

char ProfilingInstrumentationPass::ID = 0;

}  // namespace cgprofiler


static llvm::Constant*
createConstantString(llvm::Module& m, llvm::StringRef str) {
  auto& context = m.getContext();

  auto* name    = llvm::ConstantDataArray::getString(context, str, true);
  auto* int8Ty  = llvm::Type::getInt8Ty(context);
  auto* arrayTy = llvm::ArrayType::get(int8Ty, str.size() + 1);
  auto* asStr   = new llvm::GlobalVariable(
      m, arrayTy, true, llvm::GlobalValue::PrivateLinkage, name);

  auto* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value* indices[] = {zero, zero};
  return llvm::ConstantExpr::getInBoundsGetElementPtr(arrayTy, asStr, indices);
}

static DenseMap<Function*, uint64_t>
cmpt_fn_ids(llvm::ArrayRef<Function*> functions) {
  DenseMap<Function*, uint64_t> id_map;

  size_t next_id = 0;
  for (auto f : functions) {
    id_map[f] = next_id;
    std::cout << (f->getName()).str() << ": " << next_id << " "
              << "is_decl=" << f->isDeclaration() << std::endl;
    ++next_id;
  }

  return id_map;
}

void
create_fn_names(Module& m, uint64_t num_fn) {
  auto& context = m.getContext();

  auto* stringTy = Type::getInt8PtrTy(context);
  auto* tableTy  = ArrayType::get(stringTy, num_fn);

  std::vector<Constant*> values;
  for (auto it = m.begin(); it != m.end(); it++) {
    auto& f = *it;
    values.push_back(createConstantString(m, f.getName()));
  }

  auto* fn_names = ConstantArray::get(tableTy, values);
  new GlobalVariable(m,
                     tableTy,
                     false,
                     GlobalValue::ExternalLinkage,
                     fn_names,
                     "CaLlPrOfIlEr_fn_names");
}

bool
ProfilingInstrumentationPass::runOnModule(llvm::Module& m) {
  // This is the entry point of your instrumentation pass.
  auto& context = m.getContext();
  auto* int64Ty = Type::getInt64Ty(context);

  // First identify the functions we wish to track
  std::vector<Function*> all_fn;
  for (auto& f : m) {
    all_fn.push_back(&f);
  }

  // save analysis result
  fn_id_map   = cmpt_fn_ids(all_fn);
  auto num_fn = all_fn.size();
  create_fn_names(m, num_fn);

  // debug
  auto* num_fn_global = ConstantInt::get(int64Ty, num_fn, false);
  new GlobalVariable(m,
                     int64Ty,
                     true,
                     GlobalValue::ExternalLinkage,
                     num_fn_global,
                     "CaLlPrOfIlEr_num_fn");

  // register runtime fn
  auto* voidTy  = Type::getVoidTy(context);
  auto* init_fn = m.getOrInsertFunction("CaLlPrOfIlEr_init", voidTy, nullptr);
  appendToGlobalCtors(m, llvm::cast<Function>(init_fn), 0);
  auto* print_fn = m.getOrInsertFunction("CaLlPrOfIlEr_print", voidTy, nullptr);
  appendToGlobalDtors(m, llvm::cast<Function>(print_fn), 0);

  SmallVector<Type*, 3> arg_types;
  arg_types.push_back(int64Ty);
  arg_types.push_back(int64Ty);
  arg_types.push_back(int64Ty);
  auto* countTy  = FunctionType::get(voidTy, arg_types, false);
  auto* count_fn = m.getOrInsertFunction("CaLlPrOfIlEr_count", countTy);

  // insert instructions
  for (auto f : all_fn) {
    // do not change external fn
    if (f->isDeclaration()) {
      continue;
    }

    // Count each function as it is called.
    for (auto& bb : *f) {
      for (auto& i : bb) {
        handleInstruction(CallSite(&i), f, count_fn);
      }
    }
  }

  return true;
}

void
ProfilingInstrumentationPass::handleInstruction(CallSite cs,
                                                Function* caller,
                                                Value* count_fn) {
  auto instr = cs.getInstruction();
  // Check whether the instruction is actually a call
  if (!instr) {
    return;
  }

  // Check whether the called function is directly invoked
  auto callee = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (!callee) {
    return;
  }

  // // Check if the function is blacklisted.
  // if (!fn_id_map.count(callee)) {
  //   // Blacklisted functions are not counted.
  //   return;
  // }

  if ((callee->getName()).startswith(StringLiteral("llvm.dbg"))) {
    // Blacklisted functions are not counted.
    return;
  }
  auto loc = instr->getDebugLoc();

  // External functions are counted at their invocation sites.
  SmallVector<Value*, 3> args;
  IRBuilder<> builder(cs.getInstruction());
  args.push_back(builder.getInt64(fn_id_map[caller]));
  args.push_back(builder.getInt64(fn_id_map[callee]));
  args.push_back(builder.getInt64(loc->getLine()));
  builder.CreateCall(count_fn, args);
}
