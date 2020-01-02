#pragma once

#include <optional>
#include <vector>
#include <ionir/lexical/token.h>
#include <ionir/reporting/code_backtrack.h>

namespace ilc {
    class CodeTraceBlock {
    private:
        std::vector<ionir::CodeBlockLine> lines;

    public:
        static std::string createGutter(std::optional<uint32_t> lineNumber);

        static std::string createLine(std::string text, std::optional<uint32_t> lineNumber);

        static std::string createLine(ionir::CodeBlockLine line);

        explicit CodeTraceBlock(std::vector<ionir::CodeBlockLine> lines = {});

        std::vector<ionir::CodeBlockLine> &getLines();

        void setLines(std::vector<ionir::CodeBlockLine> lines);
    };
}
