#include <iostream>

#include "function.h"

LLVMTypeRef translateType(Reference* referenceM) {
  if (referenceM->ownership == Ownership::SHARE &&
      dynamic_cast<Int*>(referenceM->referend) != nullptr) {
    return LLVMInt64Type();
  } else {
    assert(false);
    return nullptr;
  }
}

std::vector<LLVMTypeRef> translateTypes(std::vector<Reference*> referencesM) {
  std::vector<LLVMTypeRef> result;
  for (auto referenceM : referencesM) {
    result.push_back(translateType(referenceM));
  }
  return result;
}

LLVMValueRef translateExpression(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    Expression* expr);
LLVMValueRef translateCall(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    Call* call);
LLVMValueRef translateExternCall(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    ExternCall* expr);


LLVMValueRef translateExternCall(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    ExternCall* call) {
  auto name = call->function->name->name;
  if (name == "F(\"__addIntInt\",[],[R(*,i),R(*,i)])") {
    assert(call->argExprs.size() == 2);
    return LLVMBuildAdd(
        builder,
        translateExpression(
            globalState, functionState, builder, call->argExprs[0]),
        translateExpression(
            globalState, functionState, builder, call->argExprs[1]),
        "add");
  } else if (name == "F(\"__addFloatFloat\",[],[R(*,f),R(*,f)])") {
    // VivemExterns.addFloatFloat
    assert(false);
  } else if (name == "F(\"panic\")") {
    // VivemExterns.panic
    assert(false);
  } else if (name == "F(\"__multiplyIntInt\",[],[R(*,i),R(*,i)])") {
    assert(call->argExprs.size() == 2);
    return LLVMBuildMul(
        builder,
        translateExpression(
            globalState, functionState, builder, call->argExprs[0]),
        translateExpression(
            globalState, functionState, builder, call->argExprs[1]),
        "mul");
  } else if (name == "F(\"__subtractIntInt\",[],[R(*,i),R(*,i)])") {
    // VivemExterns.subtractIntInt
    assert(false);
  } else if (name == "F(\"__addStrStr\",[],[R(*,s),R(*,s)])") {
    // VivemExterns.addStrStr
    assert(false);
  } else if (name == "F(\"__getch\")") {
    // VivemExterns.getch
    assert(false);
  } else if (name == "F(\"__lessThanInt\",[],[R(*,i),R(*,i)])") {
    // VivemExterns.lessThanInt
    assert(false);
  } else if (name == "F(\"__greaterThanOrEqInt\",[],[R(*,i),R(*,i)])") {
    // VivemExterns.greaterThanOrEqInt
    assert(false);
  } else if (name == "F(\"__eqIntInt\",[],[R(*,i),R(*,i)])") {
    // VivemExterns.eqIntInt
    assert(false);
  } else if (name == "F(\"__eqBoolBool\",[],[R(*,b),R(*,b)])") {
    // VivemExterns.eqBoolBool
    assert(false);
  } else if (name == "F(\"__print\",[],[R(*,s)])") {
    // VivemExterns.print
    assert(false);
  } else if (name == "F(\"__not\",[],[R(*,b)])") {
    // VivemExterns.not
    assert(false);
  } else if (name == "F(\"__castIntStr\",[],[R(*,i)])") {
    // VivemExterns.castIntStr
    assert(false);
  } else if (name == "F(\"__and\",[],[R(*,b),R(*,b)])") {
    // VivemExterns.and
    assert(false);
  } else if (name == "F(\"__mod\",[],[R(*,i),R(*,i)])") {
    // VivemExterns.mod
    assert(false);
  } else {
    assert(false);
  }
}

std::vector<LLVMValueRef> translateExpressions(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    std::vector<Expression*> exprs) {
  auto result = std::vector<LLVMValueRef>{};
  result.reserve(exprs.size());
  for (auto expr : exprs) {
    result.push_back(
        translateExpression(globalState, functionState, builder, expr));
  }
  return result;
}

LLVMValueRef translateCall(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    Call* call) {
  auto funcIter = globalState->functions.find(call->function->name->name);
  assert(funcIter != globalState->functions.end());
  auto funcL = funcIter->second;

  auto argExprsL =
      translateExpressions(globalState, functionState, builder, call->argExprs);
  return LLVMBuildCall(
      builder, funcL, argExprsL.data(), argExprsL.size(), "");
}

// A "Never" is something that should never be read.
// This is useful in a lot of situations, for example:
// - The return type of Panic()
// - The result of the Discard node
LLVMValueRef makeNever() {
  LLVMValueRef empty[1] = {};
  // We arbitrarily use a zero-len array of i57 here because it's zero sized and
  // very unlikely to be used anywhere else.
  // We could use an empty struct instead, but this'll do.
  return LLVMConstArray(LLVMIntType(57), empty, 0);
}

LLVMValueRef translateExpression(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    Expression* expr) {
  if (auto constantI64 = dynamic_cast<ConstantI64*>(expr)) {
    // See AZTMCIE for why we load and store here.
    auto localAddr = LLVMBuildAlloca(builder, LLVMInt64Type(), "");
    LLVMBuildStore(
        builder,
        LLVMConstInt(LLVMInt64Type(), constantI64->value, false),
        localAddr);
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto constantBool = dynamic_cast<ConstantBool*>(expr)) {
    // See AZTMCIE for why this is an add.
    auto localAddr = LLVMBuildAlloca(builder, LLVMInt1Type(), "");
    LLVMBuildStore(
        builder,
        LLVMConstInt(LLVMInt1Type(), constantBool->value, false),
        localAddr);
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto discard = dynamic_cast<Discard*>(expr)) {
    auto inner =
        translateExpression(
            globalState, functionState, builder, discard->sourceExpr);
    if (dynamic_cast<Int*>(discard->sourceResultType->referend) ||
        dynamic_cast<Bool*>(discard->sourceResultType->referend) ||
        dynamic_cast<Float*>(discard->sourceResultType->referend)) {
      // Do nothing for these, they're always inlined and copied.
    } else {
      std::cerr << "Unimplemented type in Discard: "
          << typeid(*discard->sourceResultType->referend).name() << std::endl;
      assert(false);
    }
    return makeNever();
  } else if (auto ret = dynamic_cast<Return*>(expr)) {
    return LLVMBuildRet(
        builder,
        translateExpression(
            globalState, functionState, builder, ret->sourceExpr));
  } else if (auto stackify = dynamic_cast<Stackify*>(expr)) {
    auto valueToStore =
        translateExpression(
            globalState, functionState, builder, stackify->sourceExpr);
    auto localAddr =
        LLVMBuildAlloca(
            builder,
            translateType(stackify->local->type),
            stackify->local->id->maybeName.c_str());
    functionState->localAddrByLocalId.emplace(
        stackify->local->id->number, localAddr);
    LLVMBuildStore(builder, valueToStore, localAddr);
    LLVMValueRef empty[1] = {};
    return LLVMConstArray(LLVMInt64Type(), empty, 0);
  } else if (auto localStore = dynamic_cast<LocalStore*>(expr)) {
    // The purpose of LocalStore is to put a swap value into a local, and give
    // what was in it.
    auto localAddr = functionState->getLocalAddr(*localStore->local->id);
    auto oldValue =
        LLVMBuildLoad(builder, localAddr, localStore->localName.c_str());
    auto valueToStore =
        translateExpression(
            globalState, functionState, builder, localStore->sourceExpr);
    LLVMBuildStore(builder, valueToStore, localAddr);
    return oldValue;
  } else if (auto localLoad = dynamic_cast<LocalLoad*>(expr)) {
    auto localAddr = functionState->getLocalAddr(*localLoad->local->id);
    return LLVMBuildLoad(builder, localAddr, localLoad->localName.c_str());
  } else if (auto unstackify = dynamic_cast<Unstackify*>(expr)) {
    // The purpose of Unstackify is to destroy the local and give what was in
    // it, but in LLVM there's no instruction (or need) for destroying a local.
    // So, we just give what was in it. It's ironically identical to LocalLoad.
    auto localAddr = functionState->getLocalAddr(*unstackify->local->id);
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto call = dynamic_cast<Call*>(expr)) {
    return translateCall(globalState, functionState, builder, call);
  } else if (auto externCall = dynamic_cast<ExternCall*>(expr)) {
    return translateExternCall(globalState, functionState, builder, externCall);
  } else if (auto argument = dynamic_cast<Argument*>(expr)) {
    return LLVMGetParam(functionState->containingFunc, argument->argumentIndex);
  } else if (auto block = dynamic_cast<Block*>(expr)) {
    auto exprs =
        translateExpressions(globalState, functionState, builder, block->exprs);
    assert(!exprs.empty());
    return exprs.back();
  } else if (auto iff = dynamic_cast<If*>(expr)) {
    auto conditionExpr =
        translateExpression(
            globalState, functionState, builder, iff->conditionExpr);

    LLVMBasicBlockRef thenBlockL =
        LLVMAppendBasicBlock(
            functionState->containingFunc,
            functionState->nextBlockName().c_str());
    LLVMBuilderRef thenBlockBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(thenBlockBuilder, thenBlockL);
    auto thenExpr =
        translateExpression(
            globalState, functionState, thenBlockBuilder, iff->thenExpr);

    LLVMBasicBlockRef elseBlockL =
        LLVMAppendBasicBlock(
            functionState->containingFunc,
            functionState->nextBlockName().c_str());
    LLVMBuilderRef elseBlockBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(elseBlockBuilder, elseBlockL);
    auto elseExpr =
        translateExpression(
            globalState, functionState, elseBlockBuilder, iff->elseExpr);

    LLVMBuildCondBr(builder, conditionExpr, thenBlockL, elseBlockL);

    LLVMBasicBlockRef afterwardBlockL =
        LLVMAppendBasicBlock(
            functionState->containingFunc,
            functionState->nextBlockName().c_str());
    LLVMBuildBr(thenBlockBuilder, afterwardBlockL);
    LLVMBuildBr(elseBlockBuilder, afterwardBlockL);

    LLVMPositionBuilderAtEnd(builder, afterwardBlockL);
    auto phi =
        LLVMBuildPhi(builder, translateType(iff->commonSupertype), "");
    LLVMValueRef incomingValueRefs[2] = { thenExpr, elseExpr };
    LLVMBasicBlockRef incomingBlocks[2] = { thenBlockL, elseBlockL };
    LLVMAddIncoming(phi, incomingValueRefs, incomingBlocks, 2);

    return phi;
  } else {
    std::string name = typeid(*expr).name();
    std::cout << name << std::endl;
    assert(false);
  }
  assert(false);
}

LLVMValueRef declareFunction(
    GlobalState* globalState,
    LLVMModuleRef mod,
    Function* functionM) {

  auto paramTypesL = translateTypes(functionM->prototype->params);
  auto returnTypeL = translateType(functionM->prototype->returnType);
  auto nameL = functionM->prototype->name->name;

  LLVMTypeRef functionTypeL =
      LLVMFunctionType(returnTypeL, paramTypesL.data(), paramTypesL.size(), 0);
  LLVMValueRef functionL = LLVMAddFunction(mod, nameL.c_str(), functionTypeL);

  assert(globalState->functions.count(functionM->prototype->name->name) == 0);
  globalState->functions.emplace(functionM->prototype->name->name, functionL);

  return functionL;
}

void translateFunction(
    GlobalState* globalState,
    LLVMModuleRef mod,
    Function* functionM) {

  auto functionL = globalState->getFunction(functionM);

  auto localAddrByLocalId = std::unordered_map<int, LLVMValueRef>{};

  FunctionState functionState(functionL);

  int blockNumber = functionState.nextBlockNumber++;
  auto blockName = std::string("block") + std::to_string(blockNumber);
  LLVMBasicBlockRef firstBlockL =
      LLVMAppendBasicBlock(functionState.containingFunc, blockName.c_str());

  LLVMBuilderRef bodyTopLevelBuilder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(bodyTopLevelBuilder, firstBlockL);
  // There are other builders made elsewhere for various blocks in the function,
  // but this is the one for the top level.
  // It's not always pointed at firstBlockL, it can be re-pointed to other
  // blocks at the top level.
  //
  // For example, in:
  //   fn main() {
  //     x! = 5;
  //     if (true && true) {
  //       mut x = 7;
  //     } else {
  //       mut x = 8;
  //     }
  //     println(x);
  //   }
  // There are four blocks:
  // - 1: contains `x! = 5` and `true && true`
  // - 2: contains `mut x = 7;`
  // - 3: contains `mut x = 8;`
  // - 4: contains `println(x)`
  //
  // When it's done making block 1, we'll make block 4 and `bodyTopLevelBuilder`
  // will point at that.
  //
  // The point is, this builder can change to point at other blocks on the same
  // level.
  //
  // All builders work like this, at whatever level theyre on.

  // Translate the body of the function. Can ignore the result because it's a
  // Never, because Valestrom guarantees we end function bodies in a ret.
  translateExpression(
      globalState, &functionState, bodyTopLevelBuilder, functionM->block);
}
