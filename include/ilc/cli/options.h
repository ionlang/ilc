#pragma once

#include <optional>
#include <set>

namespace ilc {
    enum class PhaseKind {
        Lexing,

        Parsing,

        CodeGeneration
    };

    enum class PassKind {
        TypeChecking,

        NameResolution,

        MacroExpansion,

        IonLangLogger,

        EntryPointCheck,

        BorrowCheck
    };

    struct Options {
        PhaseKind phase = PhaseKind::CodeGeneration;

        std::set<PassKind> passes = std::set<PassKind>{
            PassKind::TypeChecking,
            PassKind::NameResolution,
            PassKind::MacroExpansion,
            PassKind::IonLangLogger,
            PassKind::EntryPointCheck,
            PassKind::BorrowCheck
        };

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
    };
}
