#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>

#include <stdio.h>
#include <assert.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
// #include <optional>
#include <unordered_map>
#include <sstream>
//#include <concepts>

#include <nlohmann/json.hpp>

#include "types.h"
#include "ast.h"
#include "instructions.h"

#include "translate.h"
#include "readjson.h"
#include "vale.h"

using std::move;

// Use provided options (triple, etc.) to creation a machine
LLVMTargetMachineRef genlCreateMachine() {
  char *err;
  LLVMTargetRef target;
  LLVMCodeGenOptLevel opt_level;
  LLVMRelocMode reloc;
  LLVMTargetMachineRef machine;

  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargetMCs();
  LLVMInitializeAllTargets();
  LLVMInitializeAllAsmPrinters();
  LLVMInitializeAllAsmParsers();

  // Find target for the specified triple
  auto triple = LLVMGetDefaultTargetTriple();
  if (LLVMGetTargetFromTriple(triple, &target, &err) != 0) {
    std::cerr << "Could not create target: " << err << std::endl;
    LLVMDisposeMessage(err);
    return NULL;
  }

  bool release = true;
  bool pic = true;
  bool library = true;

  // Create a specific target machine
  opt_level = release? LLVMCodeGenLevelAggressive : LLVMCodeGenLevelNone;
  reloc = (pic || library)? LLVMRelocPIC : LLVMRelocDefault;
  auto cpu = "generic";
  auto features = "";
  if (!(machine = LLVMCreateTargetMachine(target, triple, cpu, features, opt_level, reloc, LLVMCodeModelDefault))) {
    std::cerr << "Could not create target machine" << std::endl;
    return NULL;
  }

  return machine;
}

void compileValeCode(LLVMModuleRef mod, const char* filename) {
  std::ifstream instream(filename);
  std::string str(std::istreambuf_iterator<char>{instream}, {});

  auto programJ = json::parse(str.c_str());
  auto program = readProgram(programJ);

  GlobalState globalState;

  LLVMValueRef mainL = nullptr;
  for (auto p : program->functions) {
    auto name = p.first;
    auto function = p.second;
    LLVMValueRef entryFunctionL = declareFunction(&globalState, mod, function);
    if (function->prototype->name->name == "F(\"main\")") {
      mainL = entryFunctionL;
    }
  }
  assert(mainL != nullptr);
//  mainL = testMainL;


  for (auto p : program->functions) {
    auto name = p.first;
    auto function = p.second;
    translateFunction(&globalState, mod, function);
  }

  auto paramTypesL = std::vector<LLVMTypeRef>{
      LLVMInt64Type(),
      LLVMPointerType(LLVMPointerType(LLVMInt8Type(), 0), 0)
  };
  LLVMTypeRef functionTypeL = LLVMFunctionType(LLVMInt64Type(), paramTypesL.data(), paramTypesL.size(), 0);
  LLVMValueRef entryFunctionL = LLVMAddFunction(mod, "main", functionTypeL);
  LLVMSetLinkage(entryFunctionL, LLVMDLLExportLinkage);
  LLVMSetDLLStorageClass(entryFunctionL, LLVMDLLExportStorageClass);
  LLVMSetFunctionCallConv(entryFunctionL, LLVMX86StdcallCallConv);
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMBasicBlockRef blockL = LLVMAppendBasicBlock(entryFunctionL, "thebestblock");
  LLVMPositionBuilderAtEnd(builder, blockL);
  LLVMValueRef emptyValues[1] = {};
  LLVMBuildRet(
      builder,
//      LLVMConstInt(LLVMInt64Type(), 42, false));
      LLVMBuildCall(builder, mainL, emptyValues, 0, "valeMainCall"));

  std::cout << "Printing stuff!" << std::endl;
  std::cout << LLVMPrintModuleToString(mod) << std::endl;

  char *error = NULL;
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);
  /*
  LLVMExecutionEngineRef engine;
  error = NULL;
//    LLVMLinkInJIT();
  LLVMLinkInInterpreter();
  LLVMInitializeNativeTarget();
  if (LLVMCreateExecutionEngineForModule(&engine, mod, &error) != 0) {
      fprintf(stderr, "failed to create execution engine\n");
      abort();
  }
  if (error) {
      fprintf(stderr, "error: %s\n", error);
      LLVMDisposeMessage(error);
      exit(EXIT_FAILURE);
  }

  int numArgs = 2;
  LLVMGenericValueRef args[] = {
      LLVMCreateGenericValueOfInt(LLVMInt64Type(), 0, 0),
      LLVMCreateGenericValueOfInt(LLVMInt64Type(), 0, 0),
  };

  LLVMGenericValueRef res = LLVMRunFunction(engine, entryFunctionL, numArgs, args);

  printf("%d\n", (int)LLVMGenericValueToInt(res, 0));
  */
}
