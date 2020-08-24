#pragma once

#include <vector>
#include <ionshared/misc/helpers.h>
#include <ionlang/lexical/token.h>
#include <ionlang/construct/module.h>
#include "repl_processor.h"

namespace ilc {
    class IonLangProcessor : public ReplProcessor {
    private:
        std::vector<ionlang::Token> lex();

        ionshared::Ptr<ionlang::Module> parse(std::vector<ionlang::Token> tokens);

        void codegen(ionshared::Ptr<ionlang::Module> ast);

    public:
        IonLangProcessor(Options options, std::string input);

        void run() override;
    };
}
