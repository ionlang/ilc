#define ILC_CLI_REPL_ACTION_PREFIX ':'
#define ILC_CLI_REPL_PROMPT "> "

#include <optional>
#include <iostream>
#include <ionshared/llvm/llvm_module.h>
#include <ilc/misc/const.h>
#include <ilc/repl/repl.h>
#include <ilc/repl/ionir_processor.h>
#include <ilc/repl/processor.h>

namespace ilc {
    Repl::Repl(Options options, ActionsProvider actionsProvider) :
        options(options),
        actionsProvider(actionsProvider) {
        //
    }

    void Repl::run() {
        std::string input;
        IonLangProcessor processor = IonLangProcessor(this->options, input);

        while (true) {
            std::cout << ILC_CLI_REPL_PROMPT;
            std::cout.flush();
            std::getline(std::cin, input);

            // TODO: Trim whitespace then compare.
            // No input. Continue prompt.
            if (input.length() == 0) {
                continue;
            }
            // Check actions provider against input if applicable.
            else if (input[0] == ILC_CLI_REPL_ACTION_PREFIX) {
                std::string actionName = input.substr(1);

                if (this->actionsProvider.contains(actionName)) {
                    std::optional<Callback> action = this->actionsProvider.lookup(actionName);

                    if (!action.has_value()) {
                        throw std::runtime_error("Expected action to be set");
                    }

                    // Invoke the action.
                    (*action)();
                }
                else {
                    std::cout << "Unrecognized action. Type ':quit' to exit." << std::endl;
                }

                continue;
            }

            std::cout << "--- Input: ("
                << input.length()
                << " character(s)) ---\n"
                << input
                << std::endl;

            processor.input = input;
            processor.run();
        }
    }

    ActionsProvider &Repl::getActionsProvider() {
        return this->actionsProvider;
    }
}
