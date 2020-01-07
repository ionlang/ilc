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

        /**
         * Whether to throw exceptions caught within
         * REPL mode.
         */
        bool replThrow = false;

        /**
         * Whether the input language being processed
         * is IonIR.
         */
        bool isIr = false;

        /**
         * Whether to create and run the semantic
         * analyzer pass against the parser's resulting
         * AST.
         */
        bool passSemantic = true;
    };
}
