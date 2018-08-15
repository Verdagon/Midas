/** Intermediate Representation (IR) structures and macros
*
* The IR, together with the name table, is the skeleton of the compiler.
* It connects together every stage:
*
* - The parser transforms programs into IR
* - The semantic analysis walks the IR nodes over multiple passes
* - Macro and template expansion happens via IR cloning
* - Generation transforms IR into LLVM IR
*
* The IR is comprised of heterogeneous nodes that share common INodeHdr info.
* In some cases, it is possible for several distinct node types to share an 
* identical data structure (e.g., statement expression and return).
*
* This include file will pull in the include files for all node types, including types.
* It also defines the structures needed for semantic analysis passes.
*
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#ifndef ir_h
#define ir_h

#include <llvm-c/Core.h>

#include <stdint.h>

#include "../shared/memory.h"
#include "nodes.h"
#include "namespace.h"
typedef struct Name Name;		// ../nametbl.h
typedef struct Lexer Lexer;		// ../../parser/lexer.h
typedef struct PassState PassState;

// Interfaces & headers shared across nodes
#include "inode.h"

// Typed Node header, offering access to the node's type info
// - vtype is the value type for an expression (e.g., 'i32')
#define ITypedNodeHdr \
	INodeHdr; \
	INode *vtype

// Castable structure for all typed nodes
typedef struct ITypedNode {
	ITypedNodeHdr;
} ITypedNode;

// Named Node header, for variable and type declarations
// - namesym points to the global name table entry (holds name string)
// - owner is the namespace node this name belongs to
#define INamedNodeHdr \
	ITypedNodeHdr; \
	Name *namesym; \
	struct INamedNode *owner

// Castable structure for all named nodes
typedef struct INamedNode {
	INamedNodeHdr;
} INamedNode;


#include "types/type.h"
#include "types/methtype.h"
#include "types/permission.h"
#include "types/fnsig.h"
#include "types/number.h"
#include "types/pointer.h"
#include "types/struct.h"
#include "types/array.h"
#include "types/alloc.h"

#include "stmt/module.h"
#include "stmt/block.h"

#include "exp/nameuse.h"
#include "exp/expr.h"
#include "exp/copyexpr.h"
#include "exp/fncall.h"
#include "exp/fndcl.h"
#include "exp/vardcl.h"
#include "exp/literal.h"

#include "../std/stdlib.h"

// The semantic analysis passes performed in between parse and generation
enum Passes {
	// Scope all declared names and resolve all name uses accordingly
	NameResolution,
	// Do return inference and type inference/checks.
	TypeCheck
};

// Context used across all semantic analysis passes
typedef struct PassState {
	int pass;				// Passes
	ModuleNode *mod;		// Current module
	FnSigNode *fnsig;	// The type signature of the function we are within

	int16_t scope;			// The current block scope (0=global, 1=fnsig, 2+=blocks)
	uint16_t flags;
} PassState;

#define PassWithinWhile 0x0001

#endif