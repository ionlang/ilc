#pragma once

#include <vector>
#include <optional>
#include <string>
#include <ionshared/error_handling/notice.h>
#include <ionlang/error_handling/code_backtrack.h>
#include <ionlang/lexical/token.h>

namespace ilc {
    struct StackTraceBaseOpts {
        const ionshared::Ptr<ionshared::DiagnosticStack> diagnosticStack;

        bool highlight = true;
    };

    struct IonIrStackTraceOpts : public StackTraceBaseOpts {
        ionlang::CodeBacktrack codeBacktrack;
    };

    // TODO: Make use of it.
    struct IonLangStackTraceOpts : public StackTraceBaseOpts {
        ionlang::CodeBacktrack codeBacktrack;
    };

    class StackTraceFactory {
    public:
        static std::string createGutter(std::optional<uint32_t> lineNumber);

        static std::string createLine(std::string text, std::optional<uint32_t> lineNumber);

        static std::string createLine(ionlang::CodeBlockLine line);

        static std::optional<std::string> makeCodeBlock(
            std::vector<ionlang::CodeBlockLine> codeBlock,
            bool highlight = true
        );

        static std::optional<std::string> makeStackTrace(IonIrStackTraceOpts options);
    };
}
