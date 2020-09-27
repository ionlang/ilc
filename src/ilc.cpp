#include <queue>
#include <CLI11/CLI11.hpp>
#include <ionlang/misc/static_init.h>
#include <ionir/construct/type/void_type.h>
#include <ionir/construct/prototype.h>
#include <ilc/repl/repl.h>

#define ILC_CLI_COMMAND_TRACE "trace"
#define ILC_CLI_COMMAND_REPL "repl"
#define ILC_CLI_COMMAND_VERSION "version"
#define ILC_CLI_VERSION "1.0.0"

using namespace ilc;

void setup(CLI::App &app, Options &options) {
    std::vector<std::string> inputFiles = {};

    auto versionCommand = app.add_subcommand(std::string("--") + ILC_CLI_COMMAND_VERSION, "Display the program's version");

    app.add_option("--passes", [&](std::vector<std::string> passes) {
        if (!passes.empty()) {
            options.passes.clear();
        }

        for (const auto &pass : passes) {
            if (pass == "type-check") {
                options.passes.insert(PassKind::TypeChecking);
            }
            else if (pass == "name-resolution") {
                options.passes.insert(PassKind::NameResolution);
            }
            else if (pass == "macro-expansion") {
                options.passes.insert(PassKind::MacroExpansion);
            }
            else if (pass == "borrow-check") {
                options.passes.insert(PassKind::BorrowCheck);
            }
            else {
                std::cout << "Unknown pass '" << pass << "'" << std::endl;

                return false;
            }
        }

        return true;
    });

    app.add_option("files", inputFiles, "Input files to process");

    // Register global option(s).
    // TODO: Bind '--phase' option to a sub-command 'inspect'?
    app.add_option("-p,--phase", options.phase);
    app.add_option("-o,--out", options.out);
    app.add_option("-l,--stack-trace-highlight", options.stackTraceHighlight);

    // Register sub-command: trace.
    CLI::App *trace = app.add_subcommand(ILC_CLI_COMMAND_TRACE, "Trace resulting abstract syntax tree (AST)");

    app.add_flag("-r,--repl", options.repl);
    app.add_flag("-t,--repl-throw", options.replThrow);
}

int main(int argc, char **argv) {
    CLI::App app{"Ion & IonIR command-line utility"};
    Options options{};

    setup(app, options);

    // Parse arguments.
    CLI11_PARSE(app, argc, argv);

    // Invoke static initialization of dependencies.
    ionlang::static_init::init();

    if (options.repl) {
        // TODO: Handle '--ir' flag in REPL mode.
        Repl repl = Repl(options);

        // Register common actions.
        repl.getActionsProvider().registerCommon();

        // Inform the user of REPL mode, and begin the input loop.
        std::cout << "Entering REPL mode. Type ':quit' to exit." << std::endl;
        repl.run();
    }
    else if (app.get_subcommand(ILC_CLI_COMMAND_VERSION)) {
        std::cout << "v" << ILC_CLI_VERSION << std::endl;
    }
    else if (app.get_subcommand(ILC_CLI_COMMAND_TRACE)->parsed()) {
        // TODO: Hard-coded debugging test.
        ionshared::Ptr<ionir::Args> args = std::make_shared<ionir::Args>();
        ionshared::Ptr<ionir::VoidType> returnType = std::make_shared<ionir::VoidType>();

        // TODO: Module is nullptr.
        ionshared::Ptr<ionir::Prototype> prototype = std::make_shared<ionir::Prototype>("foobar", args, returnType, nullptr);

        std::queue<ionshared::Ptr<ionir::Construct>> childrenQueue = {};

        // Push initial child.
        childrenQueue.push(prototype->nativeCast());

        // TODO: Some kind of depth counter? Currently not working exactly as intended.

        // Begin recursive children iteration.
        while (!childrenQueue.empty()) {
            childrenQueue.pop();

            ionshared::Ptr<ionir::Construct> child = childrenQueue.back();
            ionir::Ast innerChildren = child->getChildrenNodes();

            std::cout << "-- " << (int)child->constructKind;

            // Queue inner children if applicable.
            if (!innerChildren.empty()) {
                for (const auto innerChild : innerChildren) {
                    childrenQueue.push(innerChild);
                }

                // TODO: std::cout a newline if this is a terminal node, this way we can stack '--' like a tree?
            }
            else {
                std::cout << std::endl;
            }
        }
    }

    return 0;
}
