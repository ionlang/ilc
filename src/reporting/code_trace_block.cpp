#include <ilc/reporting/code_trace_block.h>

namespace ilc {
    std::string CodeTraceBlock::createGutter(std::optional<uint32_t> lineNumber) {
        return (lineNumber.has_value() ? std::to_string(*lineNumber) : " ") + " | ";
    }

    std::string CodeTraceBlock::createLine(std::string text, std::optional<uint32_t> lineNumber) {
        return "\t" + CodeTraceBlock::createGutter(lineNumber) + text + "\n";
    }

    std::string CodeTraceBlock::createLine(ionir::CodeBlockLine line) {
        // TODO: What about CodeBlockLine's 'highlight' property?
        return CodeTraceBlock::createLine(line.text, line.lineNumber);
    }

    CodeTraceBlock::CodeTraceBlock(std::vector<ionir::CodeBlockLine> lines) : lines(lines) {
        //
    }

    std::vector<ionir::CodeBlockLine> &CodeTraceBlock::getLines() {
        return this->lines;
    }

    void CodeTraceBlock::setLines(std::vector<ionir::CodeBlockLine> lines) {
        this->lines = lines;
    }
}
