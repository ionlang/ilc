#define ILC_CLI_REPL_ACTION_PREFIX ':'
#define ILC_CLI_REPL_PROMPT "> "

#include <optional>
#include <iostream>
#include <ionshared/llvm/llvm_module.h>
#include <ionir/syntax/parser.h>
#include <ilc/misc/const.h>
#include <ilc/repl/repl.h>
#include <ilc/repl/ionir_processor.h>
#include <ilc/repl/ionlang_processor.h>

namespace ilc {
    Repl::Repl(Options options, ActionsProvider actionsProvider) :
        options(options),
        actionsProvider(actionsProvider) {
        //
    }

    void Repl::run() {
        std::string input;

        // TODO: Finish critical implementation below.
        throw std::runtime_error("Implementation not finished");

        // TODO: CRITICAL! Need to pass in module.
        IonIrProcessor ionIrProcessor = IonIrProcessor(this->options, nullptr);

        IonLangProcessor ionLangProcessor = IonLangProcessor(this->options, input);

        while (true) {
            std::cout << ILC_CLI_REPL_PROMPT;
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

            std::cout << "--- Input: " << " (" << input.length() << " character(s)) ---" << std::endl;
            std::cout << input << std::endl;

            if (this->options.isIr) {
                ionIrProcessor.setInput(input);
                ionIrProcessor.run();
            }
            else {
                ionLangProcessor.setInput(input);
                ionLangProcessor.run();
            }
        }
    }

    ActionsProvider &Repl::getActionsProvider() {
        return this->actionsProvider;
    }
}
