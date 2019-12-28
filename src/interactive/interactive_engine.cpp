#include <exception>
#include <vector>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <ionir/llvm/codegen/llvm_visitor.h>
#include <ionir/syntax/parser.h>
#include <ionir/lexical/lexer.h>
#include <ionir/llvm/module.h>
#include <ionir/misc/helpers.h>
#include <ilc/misc/const.h>
#include <ilc/interactive/interactive_engine.h>

namespace ilc {
    InteractiveEngine::InteractiveEngine(ActionsProvider actionsProvider)
        : actionsProvider(actionsProvider) {
        //
    }

    void InteractiveEngine::run() {
        std::string input;

        const std::string prompt = "> ";

        while (true) {
            std::cout << prompt;
            std::getline(std::cin, input);

            // Check actions provider against input if applicable.
            if (input.length() > 0 && input[0] == '\\') {
                std::string actionName = input.substr(1);

                if (this->actionsProvider.contains(actionName)) {
                    std::optional<Callback> action = this->actionsProvider.lookup(actionName);

                    if (!action.has_value()) {
                        throw std::runtime_error("Expected action to be set");
                    }

                    // Invoke the action.
                    (*action)();
                }
            }

            std::cout << "Input: " << input << " (" << input.length() << " character(s))" << std::endl;

            ionir::Lexer lexer = ionir::Lexer(input);
            std::vector<ionir::Token> tokens = lexer.scan();

            std::cout << "Lexer: " << tokens.size() << " token(s)" << std::endl;

            ionir::TokenStream *stream = new ionir::TokenStream(tokens);
            ionir::Parser parser = ionir::Parser(stream);

            try {
                ionir::Ptr<ionir::Construct> construct = parser.parseTopLevel();

                std::cout << "Parser: " << (int)construct->getConstructKind() << std::endl;

                try {
                    llvm::LLVMContext *llvmContext = new llvm::LLVMContext();
                    llvm::Module *llvmModule = new llvm::Module(Const::appName, *llvmContext);
                    ionir::LlvmVisitor visitor = ionir::LlvmVisitor(llvmModule);
                    ionir::Module module = ionir::Module(visitor.getModule());

                    std::cout << "LLVM code-generation:" << std::endl;
                    module.print();
                }
                catch (std::exception &exception) {
                    std::cout << "LLVM code-generation: Exception" << std::endl;
                }
            }
            catch (std::exception &exception) {
                std::cout << "Parser: Exception" << std::endl;
            }

            delete stream;
        }
    }

    ActionsProvider &InteractiveEngine::getActionsProvider() {
        return this->actionsProvider;
    }
}
