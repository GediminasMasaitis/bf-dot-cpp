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

int run_output(Module& module)
{
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto triple = getDefaultTargetTriple();

    std::string err;
    const auto target = TargetRegistry::lookupTarget(triple, err);
    if (!target)
    {
        std::cerr << "Failed to lookup target " + triple + ": " + err;
        return 1;
    }

    auto options = TargetOptions();
    auto target_machine = target->createTargetMachine
    (
      triple, "generic", "", options, llvm::Optional<llvm::Reloc::Model>()
    );

    module.setTargetTriple(triple);
    auto data_layout = target_machine->createDataLayout();
    module.setDataLayout(data_layout);

    auto filename = "output.o";
    std::error_code err_code;
    auto destination = raw_fd_ostream(filename, err_code, llvm::sys::fs::F_None);
    if (err_code)
    {
        std::cerr << "Could not open file: " << err_code.message();
        return 1;
    }

    auto pass_manager = legacy::PassManager();
    auto add_pass_error = target_machine->addPassesToEmitFile(pass_manager, destination, &destination, TargetMachine::CGFT_ObjectFile);
    if (add_pass_error)
    {
        std::cerr << "TheTargetMachine can't emit a file of this type\n";
        return 1;
    }

    pass_manager.run(module);
    destination.flush();
    std::cout << "Wrote " << filename << "\n";
}

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