#ifndef STRUCT_H_
#define STRUCT_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"
#include "globalstate.h"

//class FunctionState {
//public:
//  LLVMValueRef containingFunc;
//  std::unordered_map<int, LLVMValueRef> localAddrByLocalId;
//  int nextBlockNumber = 1;
//
//  FunctionState(
//      LLVMValueRef containingFunc_,
//      LLVMBuilderRef builder_) :
//      containingFunc(containingFunc_) {}
//};

void translateStruct(
    GlobalState* globalState,
    LLVMModuleRef mod,
    StructDefinition* structM);

LLVMValueRef declareStruct(
    GlobalState* globalState,
    LLVMModuleRef mod,
    StructDefinition* structM);

#endif