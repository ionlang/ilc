#pragma once

#include <string>
#include <optional>
#include <set>
#include <vector>

namespace ilc::cli {
    enum class PhaseLevel : uint32_t {
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
        std::vector<std::string> inputFilePaths;

        PhaseLevel phaseLevel;

        // TODO: Should be default.
        std::set<PassKind> passes = std::set<cli::PassKind>({
            cli::PassKind::TypeChecking,
            cli::PassKind::NameResolution,
            cli::PassKind::MacroExpansion,
            cli::PassKind::IonLangLogger,
            cli::PassKind::EntryPointCheck,
            cli::PassKind::BorrowCheck
        });;

        /**
         * Target file path which to write result(s) to.
         */
        std::string out;

        bool doPrintPhases;

        /**
         * Whether to emit LLVM IR instead of LLVM bitcode.
         */
        bool doLlvmIr;

        bool doDebug;

        /**
         * Whether to throw exceptions caught within
         * REPL mode.
         */
        bool doJitThrow;

        bool noColor;

        bool noVerbose;
    };

    static Options options = Options{};
}
