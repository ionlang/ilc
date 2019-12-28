#pragma once

#include <exception>
#include <map>
#include <ilc/misc/helpers.h>
#include <ilc/cli/options.h>
#include "actions_provider.h"

namespace ilc {
    class InteractiveEngine {
    protected:
        Options options;

        ActionsProvider actionsProvider;

        void tryThrow(std::exception exception);

    public:
        explicit InteractiveEngine(Options options, ActionsProvider actionsProvider = ActionsProvider());

        void run();

        ActionsProvider &getActionsProvider();
    };
};
