#pragma once

#include <string>
#include <ionshared/misc/helpers.h>
#include <ionir/lexical/lexer.h>
#include <ionir/construct/module.h>
#include "repl_processor.h"

namespace ilc {
    class IonIrProcessor : public ReplProcessor {
    private:
        std::vector<ionir::Token> lex();

        ionshared::Ptr<ionir::Module> parse(std::vector<ionir::Token> tokens);

        void codegen(ionshared::Ptr<ionir::Module> ast);

    public:
        IonIrProcessor(Options options, std::string input);

        void run() override;
    };
}
