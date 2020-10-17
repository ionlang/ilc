#pragma once

#include <vector>
#include <ionshared/misc/helpers.h>
#include <ionlang/lexical/token.h>
#include <ionlang/construct/module.h>
#include <ilc/misc/helpers.h>

namespace ilc {
    class JitDriver {
    private:
        std::string input;

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

        void tryThrow(std::exception exception);

    public:
        JitDriver() noexcept;

        void run(std::string input);
    };
}
