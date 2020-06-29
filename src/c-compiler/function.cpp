#include <iostream>

#include "translatetype.h"

#include "function.h"

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
        translateExpression(globalState, functionState, builder, call->argExprs[0]),
        translateExpression(globalState, functionState, builder, call->argExprs[1]),
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
        translateExpression(globalState, functionState, builder, call->argExprs[0]),
        translateExpression(globalState, functionState, builder, call->argExprs[1]),
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
    assert(call->argExprs.size() == 2);
    return LLVMBuildICmp(
        builder,
        LLVMIntSLT,
        translateExpression(globalState, functionState, builder, call->argExprs[0]),
        translateExpression(globalState, functionState, builder, call->argExprs[1]),
        "");
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
    result.push_back(translateExpression(globalState, functionState, builder, expr));
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

  auto argExprsL = translateExpressions(globalState, functionState, builder, call->argExprs);
  return LLVMBuildCall(builder, funcL, argExprsL.data(), argExprsL.size(), "");
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
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt64Type(), constantI64->value, false), localAddr);
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto constantBool = dynamic_cast<ConstantBool*>(expr)) {
    // See AZTMCIE for why this is an add.
    auto localAddr = LLVMBuildAlloca(builder, LLVMInt1Type(), "");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt1Type(), constantBool->value, false), localAddr);
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto discard = dynamic_cast<Discard*>(expr)) {
    auto inner = translateExpression(globalState, functionState, builder, discard->sourceExpr);
    if (dynamic_cast<Int*>(discard->sourceResultType->referend) ||
        dynamic_cast<Bool*>(discard->sourceResultType->referend) ||
        dynamic_cast<Float*>(discard->sourceResultType->referend)) {
      // Do nothing for these, they're always inlined and copied.
    } else {
      std::cerr << "Unimplemented type in Discard: " << typeid(*discard->sourceResultType->referend).name() << std::endl;
      assert(false);
    }
    return makeNever();
  } else if (auto ret = dynamic_cast<Return*>(expr)) {
    return LLVMBuildRet(builder, translateExpression(globalState, functionState, builder, ret->sourceExpr));
  } else if (auto stackify = dynamic_cast<Stackify*>(expr)) {
    assert(functionState->localAddrByLocalId.count(stackify->local->id->number) == 0);
    auto name = std::string("r") + std::to_string(stackify->local->id->number);
    auto valueToStore = translateExpression(globalState, functionState, builder, stackify->sourceExpr);
    auto localAddr = LLVMBuildAlloca(builder, translateType(globalState, stackify->local->type), name.c_str());
    functionState->localAddrByLocalId.emplace(stackify->local->id->number, localAddr);
    LLVMBuildStore(builder, valueToStore, localAddr);
    LLVMValueRef empty[1] = {};
    return LLVMConstArray(LLVMInt64Type(), empty, 0);
  } else if (auto localStore = dynamic_cast<LocalStore*>(expr)) {
    // The purpose of LocalStore is to put a swap value into a local, and give what
    // was in it.

    auto localAddrIter = functionState->localAddrByLocalId.find(localStore->local->id->number);
    assert(localAddrIter != functionState->localAddrByLocalId.end());
    auto localAddr = localAddrIter->second;

    auto oldValue = LLVMBuildLoad(builder, localAddr, localStore->localName.c_str());

    auto valueToStore = translateExpression(globalState, functionState, builder, localStore->sourceExpr);
    LLVMBuildStore(builder, valueToStore, localAddr);

    return oldValue;
  } else if (auto localLoad = dynamic_cast<LocalLoad*>(expr)) {
    auto localAddrIter = functionState->localAddrByLocalId.find(localLoad->local->id->number);
    assert(localAddrIter != functionState->localAddrByLocalId.end());
    auto localAddr = localAddrIter->second;
    return LLVMBuildLoad(builder, localAddr, localLoad->localName.c_str());
  } else if (auto unstackify = dynamic_cast<Unstackify*>(expr)) {
    // The purpose of Unstackify is to destroy the local and give what was in it,
    // but in LLVM there's no instruction (or need) for destroying a local.
    // So, we just give what was in it. It's ironically identical to a localLoad.
    auto localAddrIter = functionState->localAddrByLocalId.find(unstackify->local->id->number);
    assert(localAddrIter != functionState->localAddrByLocalId.end());
    auto localAddr = localAddrIter->second;
    return LLVMBuildLoad(builder, localAddr, "");
  } else if (auto call = dynamic_cast<Call*>(expr)) {
    return translateCall(globalState, functionState, builder, call);
  } else if (auto externCall = dynamic_cast<ExternCall*>(expr)) {
    return translateExternCall(globalState, functionState, builder, externCall);
  } else if (auto argument = dynamic_cast<Argument*>(expr)) {
    return LLVMGetParam(functionState->containingFunc, argument->argumentIndex);
  } else if (auto newStruct = dynamic_cast<NewStruct*>(expr)) {
    auto structReferend = dynamic_cast<StructReferend*>(newStruct->resultType->referend);
    assert(structReferend);
    auto structIter = globalState->structs.find(structReferend->fullName->name);
    assert(structIter != globalState->structs.end());
    auto structL = structIter->second;

    switch (newStruct->resultType->ownership) {
      case Ownership::OWN:
        assert(false); // TODO: make a new mutable struct, with a call to malloc
        break;
      case Ownership::SHARE: {
        // TODO: make ref-counted immutable structs. These here are just inline value ones.
        // We do want small structs (<=32b) to be inline values, but the larger ones should
        // be able to be ref counted.

        auto exprs = translateExpressions(globalState, functionState, builder, newStruct->sourceExprs);

        LLVMValueRef structValueBeingInitialized = LLVMGetUndef(structL);
        for (int i = 0; i < exprs.size(); i++) {
          structValueBeingInitialized =
              LLVMBuildInsertValue(
                  builder, structValueBeingInitialized, exprs[i], i, "");
        }
        return structValueBeingInitialized;
      }
      case Ownership::BORROW:
        // Wouldn't make sense to make a new struct and expect a borrow reference out of it.
      // case Ownership::WEAK:
      default:
        assert(false);
        return nullptr;
    }
  } else if (auto block = dynamic_cast<Block*>(expr)) {
    auto exprs = translateExpressions(globalState, functionState, builder, block->exprs);
    assert(!exprs.empty());
    return exprs.back();
  } else if (auto iff = dynamic_cast<If*>(expr)) {
    // First, we compile the condition expression, into wherever we're currently building. It's
    // not really part of this mess, but we do use the resulting bit of it in the indirect-branch
    // instruction.
    auto conditionExpr = translateExpression(globalState, functionState, builder, iff->conditionExpr);

    // We already are in the "current" block (which is what `builder` is pointing at currently),
    // but we're about to make three more: "then", "else", and "afterward".
    //              .-----> then -----.
    //  current ---:                   :---> afterward
    //              '-----> else -----'
    // Right now, the `builder` is pointed at the "current" block.
    // After we're done, we'll change it to point at the "afterward" block, so that
    // subsequent instructions (after the If) can keep using the same builder, but they'll
    // be adding to the "afterward" block we're making here.

    int thenBlockNumber = functionState->nextBlockNumber++;
    auto thenBlockName = std::string("block") + std::to_string(thenBlockNumber);
    LLVMBasicBlockRef thenBlockL = LLVMAppendBasicBlock(functionState->containingFunc, thenBlockName.c_str());
    LLVMBuilderRef thenBlockBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(thenBlockBuilder, thenBlockL);

    int elseBlockNumber = functionState->nextBlockNumber++;
    auto elseBlockName = std::string("block") + std::to_string(elseBlockNumber);
    LLVMBasicBlockRef elseBlockL = LLVMAppendBasicBlock(functionState->containingFunc, elseBlockName.c_str());
    LLVMBuilderRef elseBlockBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(elseBlockBuilder, elseBlockL);

    int afterwardBlockNumber = functionState->nextBlockNumber++;
    auto afterwardBlockName = std::string("block") + std::to_string(afterwardBlockNumber);
    LLVMBasicBlockRef afterwardBlockL = LLVMAppendBasicBlock(functionState->containingFunc, afterwardBlockName.c_str());

    LLVMBuildCondBr(builder, conditionExpr, thenBlockL, elseBlockL);

    // Now, we fill in the "then" block.
    auto thenExpr = translateExpression(globalState, functionState, thenBlockBuilder, iff->thenExpr);
    // Instruction to jump to the afterward block.
    LLVMBuildBr(thenBlockBuilder, afterwardBlockL);

    // Now, we fill in the "else" block.
    auto elseExpr = translateExpression(globalState, functionState, elseBlockBuilder, iff->elseExpr);
    // Instruction to jump to the afterward block.
    LLVMBuildBr(elseBlockBuilder, afterwardBlockL);

    // Like explained above, here we're re-pointing the `builder` to point at the afterward block,
    // so that subsequent instructions (after the If) can keep using the same builder, but they'll
    // be adding to the "afterward" block we're making here.
    LLVMPositionBuilderAtEnd(builder, afterwardBlockL);

    // Now, we fill in the afterward block, to receive the result value of the then or else block,
    // whichever we just came from.
    auto phi = LLVMBuildPhi(builder, translateType(globalState, iff->commonSupertype), "");
    LLVMValueRef incomingValueRefs[2] = { thenExpr, elseExpr };
    LLVMBasicBlockRef incomingBlocks[2] = { thenBlockL, elseBlockL };
    LLVMAddIncoming(phi, incomingValueRefs, incomingBlocks, 2);

    // We're done with the "current" block, and also the "then" and "else" blocks, nobody else will
    // write to them now.
    // We re-pointed the `builder` to point at the "afterward" block, and subsequent instructions
    // after the if will keep adding to that.

    return phi;
  } else if (auto whiile = dynamic_cast<While*>(expr)) {
    // While only has a body expr, no separate condition.
    // If the body itself returns true, then we'll run the body again.

    // We already are in the "current" block (which is what `builder` is pointing at currently),
    // but we're about to make two more: "body" and "afterward".
    //              .-----> body -----.
    //  current ---'         ↑         :---> afterward
    //                       `--------'
    // Right now, the `builder` is pointed at the "current" block.
    // After we're done, we'll change it to point at the "afterward" block, so that
    // subsequent instructions (after the While) can keep using the same builder, but they'll
    // be adding to the "afterward" block we're making here.

    int bodyBlockNumber = functionState->nextBlockNumber++;
    auto bodyBlockName = std::string("block") + std::to_string(bodyBlockNumber);
    LLVMBasicBlockRef bodyBlockL = LLVMAppendBasicBlock(functionState->containingFunc, bodyBlockName.c_str());
    LLVMBuilderRef bodyBlockBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(bodyBlockBuilder, bodyBlockL);

    int afterwardBlockNumber = functionState->nextBlockNumber++;
    auto afterwardBlockName = std::string("block") + std::to_string(afterwardBlockNumber);
    LLVMBasicBlockRef afterwardBlockL = LLVMAppendBasicBlock(functionState->containingFunc, afterwardBlockName.c_str());

    // First, we make the "current" block jump into the "body" block.
    LLVMBuildBr(builder, bodyBlockL);

    // Now, we fill in the body block.
    auto bodyExpr = translateExpression(globalState, functionState, bodyBlockBuilder, whiile->bodyExpr);
    // And add a conditional branch to go to either itself, or the afterward block.
    LLVMBuildCondBr(bodyBlockBuilder, bodyExpr, bodyBlockL, afterwardBlockL);

    // Like explained above, here we're re-pointing the `builder` to point at the afterward block,
    // so that subsequent instructions (after the While) can keep using the same builder, but they'll
    // be adding to the "afterward" block we're making here.
    LLVMPositionBuilderAtEnd(builder, afterwardBlockL);

    // Nobody should use a result of a while, so we'll just return a Never.
    return makeNever();
  } else if (auto memberLoad = dynamic_cast<MemberLoad*>(expr)) {
    auto structExpr = translateExpression(globalState, functionState, builder, memberLoad->structExpr);
//    LLVMValueRef indicesForGetElementPtr[] = {
////        LLVMConstInt(LLVMInt64Type(),0, false),
//        LLVMConstInt(LLVMInt64Type(), memberLoad->memberIndex, false),
//    };
//    int numIndicesForGetElementPtr = 1;
//    auto ptrToMemberInStruct = LLVMBuildGEP(builder, structExpr, indicesForGetElementPtr, numIndicesForGetElementPtr, memberLoad->memberName.c_str());
//    return LLVMBuildLoad(builder, ptrToMemberInStruct, "");

//    return LLVMBuildAdd(builder, LLVMConstInt(LLVMInt64Type(), 1337, false), LLVMConstInt(LLVMInt64Type(), 1448, false), "");

    return LLVMBuildExtractValue(
        builder,
        structExpr,
        memberLoad->memberIndex,
        "");
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

  auto paramTypesL = translateTypes(globalState, functionM->prototype->params);
  auto returnTypeL = translateType(globalState, functionM->prototype->returnType);
  auto nameL = functionM->prototype->name->name;

  LLVMTypeRef functionTypeL = LLVMFunctionType(returnTypeL, paramTypesL.data(), paramTypesL.size(), 0);
  LLVMValueRef functionL = LLVMAddFunction(mod, nameL.c_str(), functionTypeL);

  assert(globalState->functions.count(functionM->prototype->name->name) == 0);
  globalState->functions.emplace(functionM->prototype->name->name, functionL);

  return functionL;
}

void translateFunction(
    GlobalState* globalState,
    Function* functionM) {

  auto functionIter = globalState->functions.find(functionM->prototype->name->name);
  assert(functionIter != globalState->functions.end());
  LLVMValueRef functionL = functionIter->second;

  auto localAddrByLocalId = std::unordered_map<int, LLVMValueRef>{};

  LLVMBuilderRef builder = LLVMCreateBuilder();
  FunctionState functionState(functionL, builder);

  int blockNumber = functionState.nextBlockNumber++;
  auto blockName = std::string("block") + std::to_string(blockNumber);
  LLVMBasicBlockRef blockL = LLVMAppendBasicBlock(functionState.containingFunc, blockName.c_str());

  LLVMPositionBuilderAtEnd(builder, blockL);

  translateExpression(globalState, &functionState, builder, functionM->block);
}
