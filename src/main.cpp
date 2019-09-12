#include <iostream>
#include <string>
#include "llvm/ADT/Optional.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;
using namespace sys;

int main(int argc, char** argv)
{
    auto context = LLVMContext();
    auto builder = IRBuilder<>(context);
    auto module = Module("mod", context);
    
    auto int_type = Type::getInt32Ty(context);
    auto int_function_type = FunctionType::get(int_type, false);

    auto main_function = Function::Create
    (
        int_function_type,
        Function::ExternalLinkage,
        "main",
        module
    );

    auto main_block = BasicBlock::Create
    (
        context,
        "main_block",
        main_function
    );

    builder.SetInsertPoint(main_block);
    builder.CreateRet(builder.getInt32(0));

    module.print(llvm::errs(), nullptr);

    return 0;
}