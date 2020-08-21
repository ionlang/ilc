#include <sstream>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/reporting/code_highlight.h>
#include <ilc/misc/console_color.h>

namespace ilc {
    std::string StackTraceFactory::createGutter(std::optional<uint32_t> lineNumber) {
        return (lineNumber.has_value() ? std::to_string(*lineNumber) : " ") + " | ";
    }

    std::string StackTraceFactory::createLine(std::string text, std::optional<uint32_t> lineNumber) {
        return "\t" + StackTraceFactory::createGutter(lineNumber) + text + "\n";
    }

    std::string StackTraceFactory::createLine(ionir::CodeBlockLine line) {
        return StackTraceFactory::createLine(line.text, line.lineNumber);
    }

    std::optional<std::string>
    StackTraceFactory::makeCodeBlock(std::vector<ionir::CodeBlockLine> codeBlock, bool highlight) {
        if (codeBlock.empty()) {
            return std::nullopt;
        }

        std::stringstream result;

        for (auto &line : codeBlock) {
            if (highlight) {
                // Entire code should be highlighted gray by default.
                // TODO: Actually use gray color.
                line.text = ConsoleColor::white(line.text);

                // Apply syntax highlighting to the line's applicable token(s).
                for (auto &token : line.tokens) {
                    /**
                     * Note that value coating is not bound to
                     * occur; the value may remain the same.
                     */
                    std::string highlightedText = CodeHighlight::coat(token);

                    /**
                     * Split the line's text into two halves, excluding the token's
                     * value, in order to insert coated text.
                     */
                    std::string firstHalf = line.text.substr(0, token.getStartPosition());
                    std::string secondHalf = line.text.substr(token.getEndPosition() - 1);

                    // Replace the line's text with the same, possibly highlighted text.
                    line.text = firstHalf + highlightedText + secondHalf;
                }
            }

            result << StackTraceFactory::createLine(line) << std::endl;
        }

        return result.str();
    }

    std::optional<std::string> StackTraceFactory::makeStackTrace(StackTraceOpts options) {
        if (options.stackTrace.empty()) {
            return std::nullopt;
        }

        std::stringstream result;
        bool prime = true;

        for (const auto &notice : options.stackTrace) {
            if (!prime) {
                result << "\tat ";
            }
            else {
                std::optional<ionir::CodeBlock> codeBlock = options.codeBacktrack.createCodeBlockNear(notice);

                if (!codeBlock.has_value()) {
                    throw std::runtime_error("Unexpected code block to be null");
                }

                std::optional<std::string> codeBlockString = StackTraceFactory::makeCodeBlock(
                    *codeBlock,
                    options.highlight
                );

                if (!codeBlockString.has_value()) {
                    throw std::runtime_error("Unexpected code block string to be null");
                }

                result << *codeBlockString;
            }

            // Append the notice's individual trace to the stack trace.
            result << notice.createTrace() << std::endl;

            // Raise the prime flag to take effect upon next iteration.
            prime = false;
        }

        /**
         * Finally, if highlight was specified, append a reset instruction
         * at the end to clear applied formatting.
         */
        if (options.highlight) {
            result << ConsoleColor::reset;
        }

        // Return the resulting stack trace string.
        return result.str();
    }
}
