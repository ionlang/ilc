#include <string>
#include <exception>
#include <vector>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "ionir/src/llvm/codegen/llvm_visitor.h"
#include "ionir/src/syntax/lexer.h"
#include "ionir/src/parsing/parser.h"
#include "ionir/src/llvm/module.h"

int main(int argc, char **argv65)
{
    std::cout << "-- onyx v1.0.0" << std::endl;

    std::string input;
    const std::string prompt = "> ";

    while (true)
    {
        std::cout << prompt;
        std::getline(std::cin, input);
        std::cout << "Input: " << input << " (" << input.size() << " character(s))" << std::endl;

        ionir::Lexer lexer = ionir::Lexer(input);
        std::vector<ionir::Token> tokens = lexer.scan();

        std::cout << "Lexer: " << tokens.size() << " token(s)" << std::endl;

        ionir::TokenStream* stream = new ionir::TokenStream(tokens);
        ionir::Parser parser = ionir::Parser(stream);

        try {
            ionir::Ptr<ionir::Construct> construct = parser.parseTopLevel();

            std::cout << "Parser: " << (int)construct->getConstructKind() << std::endl;

            try {
                llvm::LLVMContext *llvmContext = new llvm::LLVMContext();
                llvm::Module *llvmModule = new llvm::Module("onyx-interactive", *llvmContext);
                ionir::LlvmVisitor visitor = ionir::LlvmVisitor(llvmModule);
                ionir::Module module = ionir::Module(visitor.getModule());

                std::cout << "LLVM code-generation:" << std::endl;

                module.print();
            }
            catch (std::exception exception)
            {
                std::cout << "LLVM code-generation: Exception" << std::endl;
            }
        }
        catch (std::exception exception)
        {
            std::cout << "Parser: Exception" << std::endl;
        }

        delete stream;
    }

    return 0;
}
