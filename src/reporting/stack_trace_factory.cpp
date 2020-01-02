#include <sstream>
#include <ilc/reporting/stack_trace_factory.h>
#include <ilc/reporting/code_highlight.h>
#include "code_trace_block.cpp"

namespace ilc {
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
                    token.getType(),
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
            result << CodeTraceBlock::createLine(line) << std::endl;
        }

        return result.str();
    }

    std::optional<std::string> StackTraceFactory::makeStackTrace(const ionir::StackTrace stackTrace) {
        if (stackTrace.empty()) {
            return std::nullopt;
        }

        std::stringstream result;
        bool prime = true;

        for (const auto notice : stackTrace) {
            if (!prime) {
                result << "\tat ";
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
