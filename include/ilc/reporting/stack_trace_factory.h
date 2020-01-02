#pragma once

#include <vector>
#include <optional>
#include <string>
#include <ionir/reporting/stack_trace.h>
#include <ionir/reporting/code_backtrack.h>

namespace ilc {
    class StackTraceFactory {
    public:
        static std::optional<std::string> makeCodeBlock(std::vector<ionir::CodeBlockLine> codeBlock);

        static std::optional<std::string> makeStackTrace(ionir::StackTrace stackTrace);
    };
}
