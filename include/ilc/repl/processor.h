#pragma once

#include <vector>
#include <ionshared/misc/helpers.h>
#include <ionlang/lexical/token.h>
#include <ionlang/construct/module.h>
#include "repl_processor.h"

namespace ilc {
    class IonLangProcessor : public ReplProcessor {
    private:
        std::optional<ionlang::TokenStream> tokenStream;

        std::vector<ionlang::Token> lex();

        ionshared::OptPtr<ionlang::Module> parse(
            std::vector<ionlang::Token> tokens,
            ionshared::Ptr<DiagnosticVector> diagnostics
        );

        void codegen(
            ionshared::Ptr<ionlang::Module> ast,
            ionshared::Ptr<DiagnosticVector> diagnostics
        );

    public:
        IonLangProcessor(Options options, std::string input);

        void run() override;
    };
}
