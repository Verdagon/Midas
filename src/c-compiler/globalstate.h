#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <llvm-c/Core.h>

#include <unordered_map>

#include "ast.h"
#include "instructions.h"

class GlobalState {
public:
  std::unordered_map<std::string, LLVMValueRef> structs;
  std::unordered_map<std::string, LLVMValueRef> functions;
};

#endif