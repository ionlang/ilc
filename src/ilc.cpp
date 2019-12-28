#include <optional>
#include <string>
#include <CLI11/CLI11.hpp>
#include <ionir/misc/static_init.h>
#include <ilc/interactive/interactive_engine.h>

using namespace ilc;

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
     * Whether the input language being processed
     * is IonIR.
     */
    bool isIr = false;
};

void setup(CLI::App &app, Options &options) {
    // Register options.
    app.add_option("-p,--phase", options.phase);
    app.add_option("-o,--out", options.out);

    // Register flags.
    app.add_flag("-i,--interactive", options.isInteractive);
    app.add_flag("-r,--ir", options.isIr);
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
        InteractiveEngine interactiveEngine = InteractiveEngine();

        // Register common actions.
        interactiveEngine.getActionsProvider().registerCommon();

        // Inform the user of interactive mode, and begin the input loop.
        std::cout << "Entering interactive mode. Type '\\quit' to quit." << std::endl;
        interactiveEngine.run();
    }

    return 0;
}
