/** Handling for function/method declaration nodes
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"
#include "../../shared/memory.h"
#include "../../parser/lexer.h"
#include "../nametbl.h"
#include "../../shared/error.h"

#include <string.h>
#include <assert.h>

// Create a new name declaraction node
FnDclNode *newFnDclNode(Name *namesym, uint16_t flags, INode *type, INode *val) {
	FnDclNode *name;
	newNode(name, FnDclNode, FnDclTag);
    name->flags = flags;
	name->vtype = type;
	name->owner = NULL;
	name->namesym = namesym;
	name->value = val;
	name->llvmvar = NULL;
    name->nextnode = NULL;
	return name;
}

// Serialize a function node
void fnDclPrint(FnDclNode *name) {
	inodeFprint("fn %s ", &name->namesym->namestr);
	inodePrintNode(name->vtype);
	if (name->value) {
		inodeFprint(" {} ");
		if (name->value->tag == BlockTag)
			inodePrintNL();
		inodePrintNode(name->value);
	}
}

// Syntactic sugar: Turn last statement implicit returns into explicit returns
void fnImplicitReturn(INode *rettype, BlockNode *blk) {
	INode *laststmt;
	laststmt = nodesLast(blk->stmts);
	if (rettype == voidType) {
		if (laststmt->tag != ReturnTag)
			nodesAdd(&blk->stmts, (INode*) newReturnNode());
	}
	else {
		// Inject return in front of expression
		if (isExpNode(laststmt)) {
			ReturnNode *retnode = newReturnNode();
			retnode->exp = laststmt;
			nodesLast(blk->stmts) = (INode*)retnode;
		}
		else if (laststmt->tag != ReturnTag)
			errorMsgNode(laststmt, ErrorNoRet, "A return value is expected but this statement cannot give one.");
	}
}

// Enable resolution of fn parameter references to parameters
void fnDclNameResolve(PassState *pstate, FnDclNode *name) {
	int16_t oldscope = pstate->scope;
	pstate->scope = 1;
	FnSigNode *fnsig = (FnSigNode*)name->vtype;
    nametblHookPush();
    INode **nodesp;
    uint32_t cnt;
    for (nodesFor(fnsig->parms, cnt, nodesp))
        nametblHookNode((INamedNode *)*nodesp);
	inodeWalk(pstate, &name->value);
	nametblHookPop();
	pstate->scope = oldscope;
}

// Provide parameter and return type context for type checking function's logic
void fnDclTypeCheck(PassState *pstate, FnDclNode *varnode) {
	FnSigNode *oldfnsig = pstate->fnsig;
	pstate->fnsig = (FnSigNode*)varnode->vtype;
	inodeWalk(pstate, &varnode->value);
	pstate->fnsig = oldfnsig;
}

// Check the function declaration node
void fnDclPass(PassState *pstate, FnDclNode *name) {
	inodeWalk(pstate, &name->vtype);
	INode *vtype = typeGetVtype(name->vtype);

	// Process nodes in name's initial value/code block
	switch (pstate->pass) {
	case NameResolution:
		// Hook into global name table if not a global owner by module
		// (because those have already been hooked by module for forward references)
		/*if (name->owner->tag != ModuleTag)
			namespaceHook((INamedNode*)name, name->namesym);*/
		if (name->value)
			fnDclNameResolve(pstate, name);
		break;

	case TypeCheck:
		if (name->value) {
			// Syntactic sugar: Turn implicit returns into explicit returns
			fnImplicitReturn(((FnSigNode*)name->vtype)->rettype, (BlockNode *)name->value);
			// Do type checking of function (with fnsig as context)
			fnDclTypeCheck(pstate, name);
		}
		else if (vtype == voidType)
			errorMsgNode((INode*)name, ErrorNoType, "Name must specify a type");
		break;
	}
}
