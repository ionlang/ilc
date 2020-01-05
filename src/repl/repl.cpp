#define ILC_CLI_REPL_ACTION_PREFIX ':'
#define ILC_CLI_REPL_PROMPT "> "

#include <optional>
#include <vector>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <ionir/llvm/codegen/llvm_visitor.h>
#include <ionir/llvm/module.h>
#include <ionir/syntax/parser.h>
#include <ionir/lexical/lexer.h>
#include <ionir/misc/helpers.h>
#include <ilc/misc/const.h>
#include <ilc/repl/repl.h>
#include <ilc/reporting/stack_trace_factory.h>

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

            ionir::TokenStream *stream = new ionir::TokenStream(tokens);
            ionir::Parser parser = ionir::Parser(stream);

            try {
                std::optional<ionir::Ptr<ionir::Construct>>
                    construct = parser.parseTopLevel();

                // TODO: Improve if block?
                if (construct.has_value()) {
                    std::cout << "--- Parser: " << (int)construct->get()->getConstructKind() << " ---" << std::endl;
                }
                else {
                    std::cout << "Parser: [Exception] Could not parse top-level construct" << std::endl;

                    ionir::StackTrace stackTrace = parser.getStackTrace();
                    ionir::CodeBacktrack codeBacktrack = ionir::CodeBacktrack(input, *stream);

                    std::optional<std::string> stackTraceResult = StackTraceFactory::makeStackTrace(StackTraceOpts{
                        codeBacktrack,
                        stackTrace,
                        this->options.stackTraceHighlight
                    });

                    // TODO: Check for null ->make().
                    if (stackTraceResult.has_value()) {
                        std::cout << *stackTraceResult;
                    }
                    else {
                        std::cout << "Could not create stack-trace" << std::endl;
                    }

                    // TODO: Being repetitive.
                    delete stream;

                    continue;
                }

                try {
                    llvm::LLVMContext *llvmContext = new llvm::LLVMContext();
                    llvm::Module *llvmModule = new llvm::Module(Const::appName, *llvmContext);
                    ionir::LlvmVisitor visitor = ionir::LlvmVisitor(llvmModule);
                    ionir::Module module = ionir::Module(visitor.getModule());

                    // Visit the parsed top-level construct.
                    visitor.visit(*construct);

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

            delete stream;
        }
    }

    ActionsProvider &Repl::getActionsProvider() {
        return this->actionsProvider;
    }
}
