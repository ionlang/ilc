#pragma once

#include <string>
#include <exception>
#include <map>
#include <ilc/misc/helpers.h>
#include <ilc/cli/options.h>
#include "actions_provider.h"

namespace ilc {
    class Repl {
    private:
        Options options;

        ActionsProvider actionsProvider;

    public:
        explicit Repl(
            Options options,
            ActionsProvider actionsProvider = ActionsProvider()
        );

        void run();

        ActionsProvider &getActionsProvider();
    };
};
