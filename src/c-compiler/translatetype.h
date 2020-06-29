#ifndef TRANSLATETYPE_H_
#define TRANSLATETYPE_H_

#include <vector>

#include <llvm-c/Core.h>

#include "globalstate.h"

LLVMTypeRef translateType(GlobalState* globalState, Reference* referenceM);

std::vector<LLVMTypeRef> translateTypes(
    GlobalState* globalState, std::vector<Reference*> referencesM);

#endif