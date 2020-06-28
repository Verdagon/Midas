#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"
#include "globalstate.h"

class FunctionState {
public:
  LLVMValueRef containingFunc;
  std::unordered_map<int, LLVMValueRef> localAddrByLocalId;
  int nextBlockNumber = 1;

  FunctionState(
      LLVMValueRef containingFunc_,
      LLVMBuilderRef builder_) :
      containingFunc(containingFunc_) {}
};

void translateFunction(
    GlobalState* globalState,
    LLVMModuleRef mod,
    Function* functionM);

LLVMValueRef declareFunction(
    GlobalState* globalState,
    LLVMModuleRef mod,
    Function* functionM);

#endif