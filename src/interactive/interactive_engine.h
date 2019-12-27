#pragma once

#include <map>
#include "_misc/helpers.h"
#include "actions_provider.h"

namespace ilc {
    class InteractiveEngine {
    protected:
        ActionsProvider actionsProvider;

    public:
        explicit InteractiveEngine(ActionsProvider actionsProvider = ActionsProvider());

        void run();

        ActionsProvider &getActionsProvider();
    };
};
