#include <queue>
#include <CLI11/CLI11.hpp>
#include <ionir/misc/static_init.h>
#include <ilc/interactive/interactive_engine.h>
#include <ilc/cli/options.h>
#include <ionir/construct/prototype.h>

using namespace ilc;

void setup(CLI::App &app, Options &options) {
    // Register options.
    app.add_option("-p,--phase", options.phase);
    app.add_option("-o,--out", options.out);

    // Register flags.
    app.add_flag("-i,--interactive", options.isInteractive);
    app.add_flag("-t,--throw-interactive", options.throwInteractiveModeExceptions);
    app.add_flag("-r,--ir", options.isIr);

    // Register sub-command: trace.
    // TODO: Use constant name.
    CLI::App *trace = app.add_subcommand("trace", "Trace resulting abstract syntax tree (AST).");
}

int main(int argc, char **argv) {
    CLI::App app{"Ion & IonIR command-line utility"};
    Options options{};

    setup(app, options);

    // Parse arguments.
    CLI11_PARSE(app, argc, argv);

    // Invoke IonIR's static initialization.
    ionir::StaticInit::init();

    if (options.isInteractive) {
        // TODO: Handle '--ir' flag in interactive mode.
        InteractiveEngine interactiveEngine = InteractiveEngine(options);

        // Register common actions.
        interactiveEngine.getActionsProvider().registerCommon();

        // Inform the user of interactive mode, and begin the input loop.
        std::cout << "Entering interactive mode. Type '\\quit' to quit." << std::endl;
        interactiveEngine.run();
    }
        // TODO: Use constant name.
    else if (app.get_subcommand("trace")) {
        auto args = std::make_shared<ionir::Args>();
        auto returnType = std::make_shared<ionir::Type>("void");
        auto prototype = std::make_shared<ionir::Prototype>("foobar", args, returnType);
        std::queue<ionir::Ptr<ionir::Construct>> childrenQueue = {};

        // Push initial child.
        childrenQueue.push(prototype->nativeCast());

        // TODO: Some kind of depth counter?

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
            } else {
                std::cout << std::endl;
            }
        }
    }

    return 0;
}
