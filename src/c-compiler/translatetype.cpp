#include <iostream>

#include "translatetype.h"

LLVMTypeRef translateType(GlobalState* globalState, Reference* referenceM) {
  if (referenceM->ownership == Ownership::SHARE && dynamic_cast<Int*>(referenceM->referend) != nullptr) {
    return LLVMInt64Type();
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
