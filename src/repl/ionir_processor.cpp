#include <ionshared/llvm/llvm_module.h>
#include <ionlang/passes/semantic/name_resolution_pass.h>
#include <ionlang/misc/util.h>
#include <ionir/syntax/parser.h>
#include <ionir/error_handling/code_backtrack.h>
#include <ionir/passes/codegen/llvm_codegen_pass.h>
#include <ionir/passes/type_system/type_check_pass.h>
#include <ionir/passes/optimization/dead_code_elimination_pass.h>
#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ilc/passes/ionir/ionir_directive_processor_pass.h>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/repl/ionir_processor.h>

namespace ilc {
    IonIrProcessor::IonIrProcessor(Options options, ionshared::Ptr<ionir::Module> module) :
        module(module),

        // TODO: Input.
        ReplProcessor(options, "TODO") {
        //
    }

    void IonIrProcessor::run() {
        try {
            auto module = this->module;

            // TODO: Creating mock AST?
            ionir::Ast ast = {
                module
            };

            /**
             * Create a pass manager instance & run applicable passes
             * over the resulting AST.
             */
            ionir::PassManager passManager = ionir::PassManager();

            // Register all passes to be used by the pass manager.
            passManager.registerPass(std::make_shared<IonIrLoggerPass>());
            passManager.registerPass(std::make_shared<IonIrDirectiveProcessorPass>());

            // TODO
            //            passManager.registerPass(std::make_shared<ionir::NameResolutionPass>());

            passManager.registerPass(std::make_shared<ionir::DeadCodeEliminationPass>());
            passManager.registerPass(std::make_shared<ionir::TypeCheckPass>());

            // TODO: Disabled for REPL mode.
            //passManager.registerPass(std::make_shared<ionir::EntryPointCheckPass>());

            // Execute the pass manager against the parser's resulting AST.
            passManager.run(ast);

            // TODO: CRITICAL: Should be used with the PassManager instance, as a normal pass instead of manually invoking the visit functions.
            ionir::LlvmCodegenPass codegenPass = ionir::LlvmCodegenPass();

            // TODO: What if multiple top-level constructs are defined in-line? Use ionir::Driver (finish it first) and use its resulting Ast. (Additional note above).
            // Visit the parsed module construct.
            codegenPass.visitModule(module);

            std::map<std::string, llvm::Module *> modules = codegenPass.getModules()->unwrap();

            // Display the resulting code of all the modules.
            for (const auto &[key, value] : modules) {
                std::cout << "--- LLVM code-generation: " << key << " ---" << std::endl;
                ionshared::LlvmModule(value).print();
            }

            if (modules.empty()) {
                std::cout << "--- LLVM code-generation contained no modules ---" << std::endl;
            }
        }
        catch (std::exception &exception) {
            std::cout << "LLVM code-generation: [Exception] " << exception.what() << std::endl;
            this->tryThrow(exception);
        }
    }
}
