cmake_minimum_required(VERSION 3.11)
project(valec)
cmake_policy(VERSION 3.11)

find_package(LLVM 7.1.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
set(CMAKE_C_COMPILER /usr/local/opt/llvm@7/bin/clang)

include(FetchContent)

FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json
	GIT_TAG v3.7.3
)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
	FetchContent_Populate(json)
	add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories(
		${LLVM_INCLUDE_DIRS}
		"${CMAKE_SOURCE_DIR}/src/c-compiler/"
)
add_definitions(${LLVM_DEFINITIONS})

# output from `llvm-config --components | tr " " "\n"`
set(LLVM_LINK_COMPONENTS
		aarch64asmparser
		aarch64asmprinter
		aarch64codegen
		aarch64desc
		aarch64disassembler
		aarch64info
		aarch64utils
		aggressiveinstcombine
		all
		amdgpuasmparser
		amdgpuasmprinter
		amdgpucodegen
		amdgpudesc
		amdgpudisassembler
		amdgpuinfo
		amdgpuutils
		analysis
		armasmparser
		armasmprinter
		armcodegen
		armdesc
		armdisassembler
		arminfo
		armutils
		asmparser
		asmprinter
		binaryformat
		bitreader
		bitwriter
		bpfasmparser
		bpfasmprinter
		bpfcodegen
		bpfdesc
		bpfdisassembler
		bpfinfo
		codegen
		core
		coroutines
		coverage
		debuginfocodeview
		debuginfodwarf
		debuginfomsf
		debuginfopdb
		demangle
		dlltooldriver
		executionengine
		fuzzmutate
		globalisel
		hexagonasmparser
		hexagoncodegen
		hexagondesc
		hexagondisassembler
		hexagoninfo
		instcombine
		instrumentation
		interpreter
		ipo
		irreader
		lanaiasmparser
		lanaiasmprinter
		lanaicodegen
		lanaidesc
		lanaidisassembler
		lanaiinfo
		libdriver
		lineeditor
		linker
		lto
		mc
		mcdisassembler
		mcjit
		mcparser
		mipsasmparser
		mipsasmprinter
		mipscodegen
		mipsdesc
		mipsdisassembler
		mipsinfo
		mirparser
		msp430asmprinter
		msp430codegen
		msp430desc
		msp430info
		native
		nativecodegen
		nvptxasmprinter
		nvptxcodegen
		nvptxdesc
		nvptxinfo
		objcarcopts
		object
		objectyaml
		option
		orcjit
		passes
		powerpcasmparser
		powerpcasmprinter
		powerpccodegen
		powerpcdesc
		powerpcdisassembler
		powerpcinfo
		profiledata
		runtimedyld
		scalaropts
		selectiondag
		sparcasmparser
		sparcasmprinter
		sparccodegen
		sparcdesc
		sparcdisassembler
		sparcinfo
		support
		symbolize
		systemzasmparser
		systemzasmprinter
		systemzcodegen
		systemzdesc
		systemzdisassembler
		systemzinfo
		tablegen
		target
		transformutils
		vectorize
		windowsmanifest
		x86asmparser
		x86asmprinter
		x86codegen
		x86desc
		x86disassembler
		x86info
		x86utils
		xcoreasmprinter
		xcorecodegen
		xcoredesc
		xcoredisassembler
		xcoreinfo
)

llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})

add_executable(valec
		src/c-compiler/vale.cpp
		src/c-compiler/conec.c
		src/c-compiler/coneopts.c

		src/c-compiler/shared/error.c
		src/c-compiler/shared/fileio.c
		src/c-compiler/shared/memory.c
		src/c-compiler/shared/options.c
		src/c-compiler/shared/timer.c
		src/c-compiler/shared/utf8.c

		src/c-compiler/ir/clone.c
		src/c-compiler/ir/flow.c
		src/c-compiler/ir/iexp.c
		src/c-compiler/ir/inode.c
		src/c-compiler/ir/instype.c
		src/c-compiler/ir/itype.c
		src/c-compiler/ir/name.c
		src/c-compiler/ir/namespace.c
		src/c-compiler/ir/nametbl.c
		src/c-compiler/ir/nodelist.c
		src/c-compiler/ir/nodes.c

		src/c-compiler/ir/stmt/break.c
		src/c-compiler/ir/stmt/continue.c
		src/c-compiler/ir/stmt/fielddcl.c
		src/c-compiler/ir/stmt/fndcl.c
		src/c-compiler/ir/stmt/intrinsic.c
		src/c-compiler/ir/stmt/module.c
		src/c-compiler/ir/stmt/return.c
		src/c-compiler/ir/stmt/vardcl.c

		src/c-compiler/ir/exp/allocate.c
		src/c-compiler/ir/exp/assign.c
		src/c-compiler/ir/exp/block.c
		src/c-compiler/ir/exp/borrow.c
		src/c-compiler/ir/exp/cast.c
		src/c-compiler/ir/exp/deref.c
		src/c-compiler/ir/exp/fncall.c
		src/c-compiler/ir/exp/if.c
		src/c-compiler/ir/exp/literal.c
		src/c-compiler/ir/exp/logic.c
		src/c-compiler/ir/exp/loop.c
		src/c-compiler/ir/exp/namedval.c
		src/c-compiler/ir/exp/nameuse.c
		src/c-compiler/ir/exp/sizeof.c
		src/c-compiler/ir/exp/typelit.c
		src/c-compiler/ir/exp/vtuple.c

		src/c-compiler/ir/types/array.c
		src/c-compiler/ir/types/arrayref.c
		src/c-compiler/ir/types/enum.c
		src/c-compiler/ir/types/fnsig.c
		src/c-compiler/ir/types/lifetime.c
		src/c-compiler/ir/types/number.c
		src/c-compiler/ir/types/permission.c
		src/c-compiler/ir/types/pointer.c
		src/c-compiler/ir/types/reference.c
		src/c-compiler/ir/types/struct.c
		src/c-compiler/ir/types/ttuple.c
		src/c-compiler/ir/types/typedef.c
		src/c-compiler/ir/types/void.c

		src/c-compiler/ir/meta/genvardcl.c
		src/c-compiler/ir/meta/generic.c

		src/c-compiler/corelib/corelib.c
		src/c-compiler/corelib/corenumber.c

		src/c-compiler/parser/lexer.c
		src/c-compiler/parser/parser.c
		src/c-compiler/parser/parseflow.c
		src/c-compiler/parser/parseexpr.c
		src/c-compiler/parser/parsetype.c

		src/c-compiler/genllvm/genllvm.c
		src/c-compiler/genllvm/genlstmt.c
		src/c-compiler/genllvm/genlexpr.c
		src/c-compiler/genllvm/genlalloc.c
		src/c-compiler/genllvm/genltype.c
)

target_link_libraries(valec ${llvm_libs} nlohmann_json::nlohmann_json)
