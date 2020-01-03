#pragma once

#include <map>
#include <optional>
#include <ilc/misc/helpers.h>

namespace ilc {
    // TODO: Inherit from equivalent Container from libioncommon.
    class ActionsProvider {
    protected:
        std::map<std::string, Callback> actions;

    public:
        bool contains(std::string name);

        bool insert(std::string name, Callback action);

        std::optional<Callback> lookup(std::string name);

        void registerCommon();
    };
}
