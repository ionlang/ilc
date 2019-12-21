#include <vector>
#include <iostream>
#include "ionir/src/syntax/lexer.h"
#include "ionir/src/syntax/token.h"

int main()
{
    std::cout << "-- onyx v1.0.0" << std::endl;

    while (true)
    {
        std::cout << "> ";
        std::string input;

        std::cin >> input;

        ionir::Lexer lexer = ionir::Lexer(input);
        std::vector<ionir::Token> tokens = lexer.scan();

        for (auto token : tokens)
        {
            std::cout << token << std::endl;
        }
    }

    return 0;
}
