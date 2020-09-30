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
        std::vector<std::string> inputFilePaths = std::vector<std::string>();

        PhaseLevel phaseLevel = PhaseLevel::CodeGeneration;

        std::set<PassKind> passes;

        /**
         * Target file path which to write result(s) to.
         */
        std::string out = "build";

        /**
         * Whether to throw exceptions caught within
         * REPL mode.
         */
        bool jitThrow;

        bool noColor;

        /**
         * Whether to emit LLVM IR instead of LLVM bitcode.
         */
        bool llvmIr;

        bool debug;
    };

    static Options options = Options{};
}
