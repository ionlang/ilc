#pragma once

#include <vector>
#include <optional>
#include <string>
#include <ionir/reporting/notice.h>
#include <ionir/reporting/code_backtrack.h>
#include <ionir/lexical/token.h>

namespace ilc {
    class StackTraceFactory {
    public:
        static std::string createGutter(std::optional<uint32_t> lineNumber);

        static std::string createLine(std::string text, std::optional<uint32_t> lineNumber);

        static std::string createLine(ionir::CodeBlockLine line);

        static std::optional<std::string> makeCodeBlock(std::vector<ionir::CodeBlockLine> codeBlock);

        static std::optional<std::string> makeStackTrace(ionir::StackTrace stackTrace);
    };
}
