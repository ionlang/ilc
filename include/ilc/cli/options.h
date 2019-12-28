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
         * Whether to use interactive mode. If true,
         * no other options nor flags will be processed.
         */
        bool isInteractive = false;

        /**
         * Whether to throw exceptions caught within
         * interactive mode.
         */
        bool throwInteractiveModeExceptions = false;

        /**
         * Whether the input language being processed
         * is IonIR.
         */
        bool isIr = false;
    };
}
