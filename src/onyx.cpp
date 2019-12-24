#include <optional>
#include <string>
#include <exception>
#include <vector>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "../libs/CLI11/CLI11.hpp"
#include "ionir/src/llvm/codegen/llvm_visitor.h"
#include "ionir/src/syntax/lexer.h"
#include "ionir/src/parsing/parser.h"
#include "ionir/src/llvm/module.h"

namespace onyx {
    enum class PhaseKind {
        Lexing,

        Parsing,

        CodeGeneration
    };

    struct Options {
        PhaseKind phase = PhaseKind::CodeGeneration;

        /**
         * Target file path which to write result(s) to.
         */
        std::optional<std::string> out = std::nullopt;

        /**
         * Whether to use interactive mode. If true,
         * no other options nor flags will be processed.
         */
        bool isInteractive = false;

        /**
         * Whether the input language being processed
         * is IonIR.
         */
        bool isIr = false;
    };

    CLI::App setup(Options &options) {
        CLI::App app{"Ion & IonIR CLI utility"};

        // Register options.
        app.add_option("-p,--phase", options.phase);
        app.add_option("-o,--out", options.out);

        // Register flags.
        app.add_flag("-i,--interactive", options.isInteractive);
        app.add_flag("-r,--ir", options.isIr);

        return app;
    }

    void interactiveMode() {
        std::string input;
        const std::string prompt = "> ";

        while (true) {
            std::cout << prompt;
            std::getline(std::cin, input);
            std::cout << "Input: " << input << " (" << input.size() << " character(s))" << std::endl;

            ionir::Lexer lexer = ionir::Lexer(input);
            std::vector<ionir::Token> tokens = lexer.scan();

            std::cout << "Lexer: " << tokens.size() << " token(s)" << std::endl;

            ionir::TokenStream *stream = new ionir::TokenStream(tokens);
            ionir::Parser parser = ionir::Parser(stream);

            try {
                ionir::Ptr<ionir::Construct> construct = parser.parseTopLevel();

                std::cout << "Parser: " << (int) construct->getConstructKind() << std::endl;

                try {
                    llvm::LLVMContext *llvmContext = new llvm::LLVMContext();
                    llvm::Module *llvmModule = new llvm::Module("onyx-interactive", *llvmContext);
                    ionir::LlvmVisitor visitor = ionir::LlvmVisitor(llvmModule);
                    ionir::Module module = ionir::Module(visitor.getModule());

                    std::cout << "LLVM code-generation:" << std::endl;

                    module.print();
                }
                catch (std::exception exception) {
                    std::cout << "LLVM code-generation: Exception" << std::endl;
                }
            }
            catch (std::exception exception) {
                std::cout << "Parser: Exception" << std::endl;
            }

            delete stream;
        }
    }

    int main(int argc, char **argv) {
        Options options{};
        CLI::App app = setup(options);

        // Parse arguments.
        CLI11_PARSE(app, argc, argv);

        if (options.isInteractive) {
            // TODO: Handle '--ir' flag in interactive mode.
            interactiveMode();
        }

        return 0;
    }
};
