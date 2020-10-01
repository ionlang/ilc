// Include the cross-platform header before anything else.
#include <ilc/cli/cross_platform.h>

#include <queue>
#include <filesystem>
#include <CLI11/CLI11.hpp>
#include <ionshared/misc/util.h>
#include <ionlang/misc/static_init.h>
#include <ionir/construct/type/void_type.h>
#include <ionir/construct/prototype.h>
#include <ilc/cli/log.h>
#include <ilc/jit/jit_driver.h>
#include <ilc/jit/jit.h>
#include <ilc/processing/driver.h>
#include <ilc/cli/commands.h>

#define ILC_CLI_COMMAND_TRACE "trace"
#define ILC_CLI_COMMAND_JIT "jit"
#define ILC_CLI_COMMAND_VERSION "version"
#define ILC_CLI_VERSION "1.0.0"

using namespace ilc;

void setupCli(CLI::App &app) {
    // Command(s).
    cli::traceCommand = app.add_subcommand(
        ILC_CLI_COMMAND_TRACE,
        "Trace resulting abstract syntax tree (AST)"
    );

    cli::jitCommand = app.add_subcommand(
        ILC_CLI_COMMAND_JIT,
        "Use JIT to compile code REPL-style"
    );

    // Option(s).
    app.add_option(
        "files",
        cli::options.inputFilePaths,
        "Input files to process"
    )
        ->check(CLI::ExistingFile);

//    app.add_option("-p,--passes", [&](std::vector<std::string> passes) {
//        cli::options.passes.clear();
//
//        for (const auto &pass : passes) {
//            cli::options.passes.insert(cli::PassKind::NameResolution);
//
//            // TODO: Use CLI11's check.
//            if (pass == "type-check") {
//                cli::options.passes.insert(cli::PassKind::TypeChecking);
//            }
////            else if (pass == "name-resolution") {
////            cli::options.passes.insert(cli::PassKind::NameResolution);
////            }
//            else if (pass == "macro-expansion") {
//                cli::options.passes.insert(cli::PassKind::MacroExpansion);
//            }
//            else if (pass == "borrow-check") {
//                cli::options.passes.insert(cli::PassKind::BorrowCheck);
//            }
//            else {
//                return false;
//            }
//        }
//
//        return true;
//    })
//        ->default_val("macro-expansion,name-resolution,type-check,borrow-check");

    app.add_option("-l,--phase-level", cli::options.phaseLevel)
        ->check(CLI::Range(0, 2))
        ->default_val(std::to_string((int)cli::PhaseLevel::CodeGeneration));

    app.add_option(
        "-o,--out",
        cli::options.out,
        "The directory onto which to write output files"
    )
        ->default_val("build");

    // Flag(s).
    cli::jitCommand->add_flag(
        "-t,--throw",
        cli::options.doJitThrow,
        "Throw errors instead of capturing them"
    );

    app.add_flag(
        "-c,--no-color",
        cli::options.noColor,
        "Do not print color codes"
    );

    app.add_flag(
        "-b,--no-verbose",
        cli::options.noVerbose,
        "Omit verbose messages"
    );

    app.add_flag(
        "-r,--print-phases",
        cli::options.doPrintPhases,
        "Print phases"
    );

    app.add_flag(
        "-i,--llvm-ir",
        cli::options.doLlvmIr,
        "Whether to emit LLVM IR or LLVM bitcode"
    );
}

int main(int argc, char **argv) {
    CLI::App app{"Ionlang command-line utility"};

    setupCli(app);

    // Parse arguments.
    CLI11_PARSE(app, argc, argv);

    // Static initialization(s).
    ionlang::static_init::init();

    if (cli::jitCommand->parsed()) {
        jit::registerCommonActions();

        // Inform the user how to exit JIT, and begin the input loop.
        log::info("Entering REPL mode; type '\\quit' to exit");

        log::verbose("Actions registered: " + std::to_string(jit::actions.getSize()));

        std::string input;
        JitDriver jitDriver = JitDriver();

        while (true) {
            std::cout << ConsoleColor::coat("<> ", ColorKind::ForegroundGray);
            std::cout.flush();
            std::getline(std::cin, input);

            // TODO: Throwing linker reference error.
            // Trim whitespace off input string.
//            input = ionshared::util::trim(input);

            // Input string was empty, continue to next prompt.
            if (input.length() == 0) {
                continue;
            }
            // An action is being specified.
            else if (input[0] == '\\') {
                std::string actionName = input.substr(1);

                if (jit::actions.contains(actionName)) {
                    std::optional<Callback> action = jit::actions.lookup(actionName);

                    if (!action.has_value()) {
                        throw std::runtime_error("Expected action to be set");
                    }

                    (*action)();
                }
                else {
                    log::error("Unrecognized action '" + actionName + "'; Type '\\quit' to exit");
                }

                continue;
            }

            std::cout << "--- Input: ("
                << input.length()
                << " character(s)) ---\n"
                << input
                << std::endl;

            jitDriver.run(input);
        }
    }
    else if (cli::traceCommand->parsed()) {
        // TODO: Hard-coded debugging test.
        ionshared::Ptr<ionir::Args> args = std::make_shared<ionir::Args>();
        ionshared::Ptr<ionir::VoidType> returnType = std::make_shared<ionir::VoidType>();

        // TODO: Module is nullptr.
        ionshared::Ptr<ionir::Prototype> prototype =
            std::make_shared<ionir::Prototype>("foobar", args, returnType, nullptr);

        std::queue<ionshared::Ptr<ionir::Construct>> childrenQueue = {};

        // Push initial child.
        childrenQueue.push(prototype->nativeCast());

        // TODO: Some kind of depth counter? Currently not working exactly as intended.

        // Begin recursive children iteration.
        while (!childrenQueue.empty()) {
            childrenQueue.pop();

            ionshared::Ptr<ionir::Construct> child = childrenQueue.back();
            ionir::Ast innerChildren = child->getChildrenNodes();

            std::cout << "-- "
                << child->findConstructKindName().value_or("Unknown")
                << std::endl;

            // Queue inner children if applicable.
            if (!innerChildren.empty()) {
                for (const auto innerChild : innerChildren) {
                    childrenQueue.push(innerChild);
                }

                // TODO: std::cout a newline if this is a terminal node, this way we can stack '--' like a tree?
            }
        }
    }
    else if (!cli::options.inputFilePaths.empty()) {
        log::verbose("Processing " + std::to_string(cli::options.inputFilePaths.size()) + " input file(s)");

        std::stringstream inputStringStream = std::stringstream();
        Driver driver = Driver();
        std::string outputFileExtension = std::string(".") + (cli::options.doLlvmIr ? "ll" : "o");

        // Create the output directory if it doesn't already exist.
        if (!std::filesystem::exists(cli::options.out)) {
            log::verbose("Creating output directory '" + cli::options.out + "'");

            // Ensure the directory was created, otherwise fail the process.
            if (!std::filesystem::create_directory(cli::options.out)) {
                log::error("Output directory could not be created");

                return EXIT_FAILURE;
            }
        }

        // TODO: Make target triple be taken in through options, with default to host.
        llvm::Triple targetTriple = llvm::Triple(llvm::sys::getDefaultTargetTriple());

        bool success = true;

        log::verbose("Using target triple: " + targetTriple.getTriple());

        for (const auto &inputFilePath : cli::options.inputFilePaths) {
            inputStringStream << std::ifstream(inputFilePath).rdbuf();

            std::filesystem::path outputFilePath =
                std::filesystem::path(cli::options.out)
                    .append(inputFilePath)
                    .concat(outputFileExtension);

            log::verbose("Generating '" + outputFilePath.string() + "'");

            // Stop processing input files if the driver fails to run.
            if (!driver.run(targetTriple, outputFilePath, inputStringStream.str())) {
                success = false;

                break;
            }
        }

        if (!success) {
            log::error("Generation failed");
        }
    }
    else {
        log::error("No input files; Use --help to view commands");
    }

    return EXIT_SUCCESS;
}
