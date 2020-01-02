#include <exception>
#include <sstream>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/reporting/code_highlight.h>

namespace ilc {
    std::string StackTraceFactory::createGutter(std::optional<uint32_t> lineNumber) {
        return (lineNumber.has_value() ? std::to_string(*lineNumber) : " ") + " | ";
    }

    std::string StackTraceFactory::createLine(std::string text, std::optional<uint32_t> lineNumber) {
        return "\t" + StackTraceFactory::createGutter(lineNumber) + text + "\n";
    }

    std::string StackTraceFactory::createLine(ionir::CodeBlockLine line) {
        // TODO: What about CodeBlockLine's 'highlight' property?
        return StackTraceFactory::createLine(line.text, line.lineNumber);
    }

    std::optional<std::string> StackTraceFactory::makeCodeBlock(std::vector<ionir::CodeBlockLine> codeBlock) {
        if (codeBlock.empty()) {
            return std::nullopt;
        }

        std::stringstream result;

        for (auto line : codeBlock) {
            // Apply syntax highlighting to the line's applicable token(s).
            for (auto &token : line.tokens) {
                /**
                 * Replace entire token with coated value,
                 * since tokens' properties are read-only.
                 * Note that value coating is not bound to
                 * occur; the value may remain the same.
                 */
                token = ionir::Token(
                    token.getKind(),
                    CodeHighlight::coat(token),
                    token.getStartPosition(),
                    token.getLineNumber()
                );
            }

            /**
             * TODO: Replacing token with coated one
             * is currently useless since final code
             * will be based upon the 'text' property
             * instead of the token vector of 'line'.
             */
            result << StackTraceFactory::createLine(line) << std::endl;
        }

        return result.str();
    }

    std::optional<std::string>
    StackTraceFactory::makeStackTrace(ionir::CodeBacktrack codeBacktrack, const ionir::StackTrace stackTrace) {
        if (stackTrace.empty()) {
            return std::nullopt;
        }

        std::stringstream result;
        bool prime = true;

        for (const auto notice : stackTrace) {
            if (!prime) {
                result << "\tat ";
            }
            else {
                std::optional<ionir::CodeBlock> codeBlock = codeBacktrack.createCodeBlockNear(notice);

                if (!codeBlock.has_value()) {
                    throw std::runtime_error("Unexpected code block to be null");
                }

                std::optional<std::string> codeBlockString = StackTraceFactory::makeCodeBlock(*codeBlock);

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

        // Return the resulting stack trace string.
        return result.str();
    }
}
