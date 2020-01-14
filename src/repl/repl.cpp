#define ILC_CLI_REPL_ACTION_PREFIX ':'
#define ILC_CLI_REPL_PROMPT "> "

#include <optional>
#include <vector>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <ionir/passes/codegen/llvm_codegen_pass.h>
#include <ionir/passes/optimization/dead_code_elimination_pass.h>
#include <ionir/passes/type_system/type_checker_pass.h>
#include <ionir/passes/pass.h>
#include <ionir/passes/pass_manager.h>
#include <ionir/llvm/llvm_module.h>
#include <ionir/syntax/parser.h>
#include <ionir/lexical/lexer.h>
#include <ionir/misc/helpers.h>
#include <ilc/misc/const.h>
#include <ilc/repl/repl.h>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/passes/directive_processor_pass.h>

namespace ilc {
    Repl::Repl(Options options, ActionsProvider actionsProvider)
        : options(options), actionsProvider(actionsProvider) {
        //
    }

    void Repl::tryThrow(std::exception exception) {
        if (this->options.replThrow) {
            throw exception;
        }
    }

    class LoggerPass : public ionir::Pass {
    public:
        void visit(ionir::Ptr<ionir::Construct> node) override {
            std::cout << "Visiting node: " << (int)node->getConstructKind() << std::endl;

            ionir::Pass::visit(node);
        }
    };

    void Repl::run() {
        std::string input;

        while (true) {
            std::cout << ILC_CLI_REPL_PROMPT;
            std::getline(std::cin, input);

            // Check actions provider against input if applicable.
            if (input.length() > 0 && input[0] == ILC_CLI_REPL_ACTION_PREFIX) {
                std::string actionName = input.substr(1);

                if (this->actionsProvider.contains(actionName)) {
                    std::optional<Callback> action = this->actionsProvider.lookup(actionName);

                    if (!action.has_value()) {
                        throw std::runtime_error("Expected action to be set");
                    }

                    // Invoke the action.
                    (*action)();
                }
                else {
                    std::cout << "Unrecognized action. Type ':quit' to exit." << std::endl;
                }

                continue;
            }

            std::cout << "--- Input: " << " (" << input.length() << " character(s)) ---" << std::endl;
            std::cout << input << std::endl;

            ionir::Lexer lexer = ionir::Lexer(input);
            std::vector<ionir::Token> tokens = lexer.scan();

            std::cout << "--- Lexer: " << tokens.size() << " token(s) ---" << std::endl;

            for (auto token : tokens) {
                std::cout << token << std::endl;
            }

            ionir::TokenStream stream = ionir::TokenStream(tokens);
            ionir::Parser parser = ionir::Parser(stream);

            try {
                std::optional<ionir::Ptr<ionir::Construct>>
                    construct = parser.parseTopLevel();

                // TODO: Improve if block?
                if (construct.has_value()) {
                    // TODO: What if multiple top-level, in-line constructs are parsed? (Additional note below).
                    std::cout << "--- Parser: " << (int)construct->get()->getConstructKind() << " ---" << std::endl;
                }
                else {
                    std::cout << "Parser: [Exception] Could not parse top-level construct" << std::endl;

                    ionir::StackTrace stackTrace = parser.getStackTrace();
                    ionir::CodeBacktrack codeBacktrack = ionir::CodeBacktrack(input, stream);

                    std::optional<std::string> stackTraceResult = StackTraceFactory::makeStackTrace(StackTraceOpts{
                        codeBacktrack,
                        stackTrace,
                        this->options.stackTraceHighlight
                    });

                    // TODO: Check for null ->make().
                    if (stackTraceResult.has_value()) {
                        std::cout << std::endl << *stackTraceResult;
                    }
                    else {
                        std::cout << "Could not create stack-trace" << std::endl;
                    }

                    continue;
                }

                try {
                    llvm::LLVMContext *llvmContext = new llvm::LLVMContext();
                    llvm::Module *llvmModule = new llvm::Module(Const::appName, *llvmContext);
                    ionir::LlvmCodegenPass llvmCodegenPass = ionir::LlvmCodegenPass(llvmModule);

                    // TODO: What if multiple top-level constructs are defined in-line? Use ionir::Driver (finish it first) and use its resulting Ast. (Additional note above).
                    // Visit the parsed top-level construct.
                    llvmCodegenPass.visit(*construct);

                    // TODO: Creating mock AST.
                    ionir::Ast ast = {
                        *construct
                    };

                    /**
                     * Re-bind llvm module pointer after
                     * visiting construct in case that a
                     * module was visited.
                     */
                    llvmModule = llvmCodegenPass.getModule();

                    ionir::LlvmModule module = ionir::LlvmModule(llvmModule);

                    /**
                     * Create a pass manager instance & run
                     * applicable passes over the resulting AST.
                     */
                    ionir::PassManager passManager = ionir::PassManager();

                    // Register all passes to be used by the pass manager.
                    passManager.registerPass(std::make_shared<LoggerPass>());
                    passManager.registerPass(std::make_shared<DirectiveProcessorPass>());
                    passManager.registerPass(std::make_shared<ionir::DeadCodeEliminationPass>());
                    passManager.registerPass(std::make_shared<ionir::TypeCheckerPass>());

                    // Execute the pass manager against the parser's resulting AST.
                    passManager.run(ast);

                    std::cout << "--- LLVM code-generation ---" << std::endl;
                    module.print();
                }
                catch (std::exception &exception) {
                    std::cout << "LLVM code-generation: [Exception] " << exception.what() << std::endl;
                    this->tryThrow(exception);
                }
            }
            catch (std::exception &exception) {
                std::cout << "Parser: [Exception] " << exception.what() << std::endl;
                this->tryThrow(exception);
            }
        }
    }

    ActionsProvider &Repl::getActionsProvider() {
        return this->actionsProvider;
    }
}
