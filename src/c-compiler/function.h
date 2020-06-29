#ifndef TRANSLATE_H_
#define TRANSLATE_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"

class GlobalState {
public:
  std::unordered_map<std::string, LLVMValueRef> functions;

  LLVMValueRef getFunction(Function* functionM) {
    auto functionIter = functions.find(functionM->prototype->name->name);
    assert(functionIter != functions.end());
    return functionIter->second;
  }
};

class FunctionState {
public:
  LLVMValueRef containingFunc;
  std::unordered_map<int, LLVMValueRef> localAddrByLocalId;
  int nextBlockNumber = 1;

  FunctionState(LLVMValueRef containingFunc_) :
      containingFunc(containingFunc_) {}

  LLVMValueRef getLocalAddr(const VariableId& varId) {
    auto localAddrIter = localAddrByLocalId.find(varId.number);
    assert(localAddrIter != localAddrByLocalId.end());
    return localAddrIter->second;
  }

  std::string nextBlockName() {
    return std::string("block") + std::to_string(nextBlockNumber++);
  }
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