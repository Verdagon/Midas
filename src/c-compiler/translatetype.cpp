#include <iostream>

#include "translatetype.h"

LLVMTypeRef translateType(GlobalState* globalState, Reference* referenceM) {
  if (dynamic_cast<Int*>(referenceM->referend) != nullptr) {
    assert(referenceM->ownership == Ownership::SHARE);
    return LLVMInt64Type();
  } else if (dynamic_cast<Bool*>(referenceM->referend) != nullptr) {
    assert(referenceM->ownership == Ownership::SHARE);
    return LLVMInt1Type();
  } else if (auto structReferend =
      dynamic_cast<StructReferend*>(referenceM->referend)) {

    bool inliine = true;//referenceM->location == INLINE; TODO

    if (inliine) {
      auto structM = globalState->program->getStruct(structReferend->fullName);

      std::vector<LLVMTypeRef> memberTypesL;
      for (auto memberM : structM->members) {
        memberTypesL.push_back(translateType(globalState, memberM->type));
      }
      auto anonymousType =
          LLVMStructType(&memberTypesL[0], memberTypesL.size(),false);
      return anonymousType;
    } else {
      auto structLIter = globalState->structs.find(structReferend->fullName->name);
      assert(structLIter != globalState->structs.end());
      auto structL = structLIter->second;
      return structL;
    }
  } else {
    std::cerr << "Unimplemented type: " << typeid(*referenceM->referend).name() << std::endl;
    assert(false);
    return nullptr;
  }
}

std::vector<LLVMTypeRef> translateTypes(GlobalState* globalState, std::vector<Reference*> referencesM) {
  std::vector<LLVMTypeRef> result;
  for (auto referenceM : referencesM) {
    result.push_back(translateType(globalState, referenceM));
  }
  return result;
}
