#include <iostream>

#include "struct.h"

#include "translatetype.h"

void declareStruct(
    GlobalState* globalState,
    StructDefinition* structM) {

  auto structL =
      LLVMStructCreateNamed(
          LLVMGetGlobalContext(), structM->name->name.c_str());

  assert(globalState->structs.count(structM->name->name) == 0);
  globalState->structs.emplace(structM->name->name, structL);
}

void translateStruct(
    GlobalState* globalState,
    StructDefinition* structM) {
  LLVMTypeRef structL = globalState->getStruct(structM->name);

  std::vector<LLVMTypeRef> memberTypesL(structM->members.size());
  for (auto member : structM->members) {
    memberTypesL.push_back(translateType(globalState, member->type));
  }
  LLVMStructSetBody(
      structL, &memberTypesL[0], memberTypesL.size(), false);
}
