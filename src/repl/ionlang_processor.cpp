#include <ionshared/error_handling/notice.h>
#include <ionshared/llvm/llvm_module.h>
#include <ionlang/lexical/lexer.h>
#include <ionlang/syntax/parser.h>
#include <ilc/repl/ionlang_processor.h>
#include <ilc/reporting/stack_trace_factory.h>

namespace ilc {
    std::vector<ionlang::Token> IonLangProcessor::lex() {
        ionlang::Lexer lexer = ionlang::Lexer(this->getInput());
        std::vector<ionlang::Token> tokens = lexer.scan();

        std::cout << "--- Lexer: " << tokens.size() << " token(s) ---" << std::endl;

        for (auto token : tokens) {
            std::cout << token << std::endl;
        }

        return tokens;
    }

    void IonLangProcessor::parse(std::vector<ionlang::Token> tokens) {
        ionlang::TokenStream stream = ionlang::TokenStream(tokens);
        ionlang::Parser parser = ionlang::Parser(stream);

        try {
            ionshared::OptPtr<ionlang::Module> moduleResult = parser.parseModule();

            // TODO: Improve if block?
            if (ionshared::Util::hasValue(moduleResult)) {
                // TODO: What if multiple top-level, in-line constructs are parsed? (Additional note below).
                std::cout << "--- Parser ---" << std::endl;
            }
            else {
                std::cout << "Parser: [Exception] Could not parse module" << std::endl;

                ionshared::Ptr<ionshared::NoticeStack> noticeStack = parser.getNoticeStack();
                ionir::CodeBacktrack codeBacktrack = ionir::CodeBacktrack(this->getInput(), stream);

                std::optional<std::string> stackTraceResult = StackTraceFactory::makeStackTrace(StackTraceOpts{
                    codeBacktrack,
                    noticeStack,
                    this->getOptions().stackTraceHighlight
                });

                // TODO: Check for null ->make().
                if (stackTraceResult.has_value()) {
                    std::cout << std::endl << *stackTraceResult;
                }
                else {
                    std::cout << "Could not create stack-trace" << std::endl;
                }

                return;
            }

            this->codegen(*moduleResult);
        }
        catch (std::exception &exception) {
            std::cout << "Parser: [Exception] " << exception.what() << std::endl;
            this->tryThrow(exception);
        }
    }

    void IonLangProcessor::codegen(ionshared::Ptr<ionlang::Module> module) {
        try {
            // TODO: Creating mock AST?
            ionlang::Ast ast = {
                module
            };

            /**
             * Create a pass manager instance & run applicable passes
             * over the resulting AST.
             */
            ionlang::PassManager passManager = ionlang::PassManager();

            // Register all passes to be used by the pass manager.
            // TODO: Create and implement IonLangLogger pass.
//            passManager.registerPass(std::make_shared<IonIrLoggerPass>());

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

    IonLangProcessor::IonLangProcessor(Options options, std::string input)
        : ReplProcessor(options, input) {
        //
    }

    void IonLangProcessor::run() {
        // TODO: Implement.
        throw std::runtime_error("Not implemented");
    }
}
