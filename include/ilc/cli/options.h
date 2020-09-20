#pragma once

#include <optional>

namespace ilc {
    enum class PhaseKind {
        Lexing,

        Parsing,

        CodeGeneration
    };

    struct Options {
        PhaseKind phase = PhaseKind::CodeGeneration;

        /**
         * Target file path which to write result(s) to.
         */
        std::optional<std::string> out = std::nullopt;

        /**
         * Whether to apply syntax-highlighting to stack trace.
         * code blocks.
         */
        bool stackTraceHighlight = true;

        bool repl = false;

        /**
         * Whether to throw exceptions caught within
         * REPL mode.
         */
        bool replThrow = false;

        /**
         * Whether to create and run the semantic
         * analyzer pass against the parser's resulting
         * AST.
         */
        bool passSemantic = true;
    };
}
