#ifndef TRANSLATE_H_
#define TRANSLATE_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"

class GlobalState {
public:
  std::unordered_map<std::string, LLVMValueRef> functions;
};

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