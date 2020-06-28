#include <iostream>

#include "struct.h"

LLVMValueRef declareStruct(
    GlobalState* globalState,
    LLVMModuleRef mod,
    StructDefinition* structM) {

  LLVMStruct

  std::vector<Reference*> memberTypes;
  for (auto member : structM->members) {
    memberTypes.push_back(member->type);
  }

  LLVMTypeRef structTypeL = LLVMStructType(returnTypeL, paramTypesL.data(), paramTypesL.size(), 0);
  LLVMValueRef structL = LLVMAddStruct(mod, nameL.c_str(), structTypeL);

  assert(globalState->structs.count(structM->prototype->name->name) == 0);
  globalState->structs.emplace(structM->prototype->name->name, structL);

  return structL;
}

void translateStruct(
    GlobalState* globalState,
    LLVMModuleRef mod,
    StructDefinition* structM) {

  auto structIter = globalState->structs.find(structM->prototype->name->name);
  assert(structIter != globalState->structs.end());
  LLVMValueRef structL = structIter->second;

  auto localAddrByLocalId = std::unordered_map<int, LLVMValueRef>{};

  LLVMBuilderRef builder = LLVMCreateBuilder();
  StructState structState(structL, builder);

  int blockNumber = structState.nextBlockNumber++;
  auto blockName = std::string("block") + std::to_string(blockNumber);
  LLVMBasicBlockRef blockL = LLVMAppendBasicBlock(structState.containingFunc, blockName.c_str());

  LLVMPositionBuilderAtEnd(builder, blockL);

  translateExpression(globalState, &structState, builder, structM->block);
}
