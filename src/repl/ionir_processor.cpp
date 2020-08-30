#include <ionshared/llvm/llvm_module.h>
#include <ionir/syntax/parser.h>
#include <ionir/error_handling/code_backtrack.h>
#include <ionir/passes/codegen/llvm_codegen_pass.h>
#include <ionir/passes/semantic/name_resolution_pass.h>
#include <ionir/passes/semantic/name_shadowing_pass.h>
#include <ionir/passes/type_system/type_check_pass.h>
#include <ionir/passes/optimization/dead_code_elimination_pass.h>
#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ilc/passes/ionir/ionir_directive_processor_pass.h>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/repl/ionir_processor.h>

namespace ilc {
    std::vector<ionir::Token> IonIrProcessor::lex() {
        ionir::Lexer lexer = ionir::Lexer(this->getInput());
        std::vector<ionir::Token> tokens = lexer.scan();

        std::cout << "--- Lexer: " << tokens.size() << " token(s) ---" << std::endl;

        for (auto token : tokens) {
            std::cout << token << std::endl;
        }

        return tokens;
    }

    ionshared::Ptr<ionir::Module> IonIrProcessor::parse(std::vector<ionir::Token> tokens) {
        ionshared::Ptr<ionir::TokenStream> tokenStream = std::make_shared<ionir::TokenStream>(tokens);
        ionir::Parser parser = ionir::Parser(tokenStream);

        try {
            ionir::AstPtrResult<ionir::Module> moduleResult = parser.parseModule();

            // TODO: Improve if block?
            if (ionir::util::hasValue(moduleResult)) {
                // TODO: What if multiple top-level, in-line constructs are parsed? (Additional note below).
                std::cout << "--- Parser ---" << std::endl;
            }
            else {
                std::cout << "Parser: [Exception] Could not parse module" << std::endl;

                ionshared::Ptr<ionshared::NoticeStack> noticeStack = parser.getNoticeStack();

                // TODO: De-referencing token stream. However, if it was a reference, then it would maintain it's index... Figure it out.
                ionir::CodeBacktrack codeBacktrack = ionir::CodeBacktrack(this->getInput(), *tokenStream);

                std::optional<std::string> stackTraceResult = StackTraceFactory::makeStackTrace(IonIrStackTraceOpts{
                    noticeStack,
                    this->getOptions().stackTraceHighlight,
                    codeBacktrack
                });

                // TODO: Check for null ->make().
                if (stackTraceResult.has_value()) {
                    std::cout << std::endl << *stackTraceResult;
                }
                else {
                    std::cout << "Could not create stack-trace" << std::endl;
                }

                // TODO: Cannot return empty. Throw for now to satisfy return.
                throw std::runtime_error("!!! DEBUGGING POINT !!!");
//                return;
            }

            ionshared::Ptr<ionir::Module> module = ionir::util::getResultValue(moduleResult);

            return ionir::util::getResultValue(moduleResult);
        }
        catch (std::exception &exception) {
            std::cout << "Parser: [Exception] " << exception.what() << std::endl;
            this->tryThrow(exception);
        }

        throw std::runtime_error("!! DEBUGGING POINT: NEEDS RETURN !!");
    }

    void IonIrProcessor::codegen(ionshared::Ptr<ionir::Module> module) {
        try {
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
            passManager.registerPass(std::make_shared<ionir::NameResolutionPass>());
            passManager.registerPass(std::make_shared<ionir::NameShadowingPass>());
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

    IonIrProcessor::IonIrProcessor(Options options, std::string input) :
        ReplProcessor(options, input) {
        //
    }

    void IonIrProcessor::run() {
        std::vector<ionir::Token> tokens = this->lex();
        ionshared::Ptr<ionir::Module> module = this->parse(tokens);

        this->codegen(module);
    }
}
