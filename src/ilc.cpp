#define ILC_CLI_COMMAND_TRACE "trace"
#define ILC_CLI_COMMAND_REPL "repl"

#include <queue>
#include <CLI11/CLI11.hpp>
#include <ionir/misc/static_init.h>
#include <ionir/construct/type/void_type.h>
#include <ionir/construct/prototype.h>
#include <ilc/repl/repl.h>
#include <ilc/cli/options.h>

using namespace ilc;

void setup(CLI::App &app, Options &options) {
    // Register global option(s).
    // TODO: Bind '--phase' option to a sub-command 'inspect'?
    app.add_option("-p,--phase", options.phase);
    app.add_option("-o,--out", options.out);
    app.add_option("-l,--stack-trace-highlight", options.stackTraceHighlight);

    // Register pass option(s).
    app.add_option("--pass-semantic", options.passSemantic);

    // Register global flag(s).
    app.add_flag("-r,--ir", options.isIr);

    // Register sub-command: trace.
    CLI::App *trace = app.add_subcommand(ILC_CLI_COMMAND_TRACE, "Trace resulting abstract syntax tree (AST)");

    // Register sub-command: repl.
    CLI::App *repl = app.add_subcommand(ILC_CLI_COMMAND_REPL, "Use interactive mode");

    repl->add_flag("-t,--repl-throw", options.replThrow);
}

int main(int argc, char **argv) {
    CLI::App app{"Ion & IonIR command-line utility"};
    Options options{};

    setup(app, options);

    // Parse arguments.
    CLI11_PARSE(app, argc, argv);

    // Invoke IonIR's static initialization.
    ionir::StaticInit::init();

    if (app.get_subcommand(ILC_CLI_COMMAND_REPL)->parsed()) {
        // TODO: Handle '--ir' flag in REPL mode.
        Repl repl = Repl(options);

        // Register common actions.
        repl.getActionsProvider().registerCommon();

        // Inform the user of REPL mode, and begin the input loop.
        std::cout << "Entering REPL mode. Type ':quit' to exit." << std::endl;
        repl.run();
    }
    else if (app.get_subcommand(ILC_CLI_COMMAND_TRACE)->parsed()) {
        // TODO: Hard-coded debugging test.
        ionir::Ptr<ionir::Args> args = std::make_shared<ionir::Args>();
        ionir::Ptr<ionir::VoidType> returnType = std::make_shared<ionir::VoidType>();
        ionir::Ptr<ionir::Prototype> prototype = std::make_shared<ionir::Prototype>("foobar", args, returnType);
        std::queue<ionir::Ptr<ionir::Construct>> childrenQueue = {};

        // Push initial child.
        childrenQueue.push(prototype->nativeCast());

        // TODO: Some kind of depth counter? Currently not working exactly as intended.

        // Begin recursive children iteration.
        while (!childrenQueue.empty()) {
            childrenQueue.pop();

            ionir::Ptr<ionir::Construct> child = childrenQueue.back();
            ionir::ConstructChildren innerChildren = child->getChildren();

            std::cout << "-- " << (int)child->getConstructKind();

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
