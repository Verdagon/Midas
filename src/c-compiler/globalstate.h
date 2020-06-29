#ifndef GLOBALSTATE_H_
#define GLOBALSTATE_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"

class GlobalState {
public:
  Program* program;
  std::unordered_map<std::string, LLVMTypeRef> structs;
  std::unordered_map<std::string, LLVMValueRef> functions;
};

#endif