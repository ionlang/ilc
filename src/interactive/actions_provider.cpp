#include "actions_provider.h"

namespace ilc {
    bool ActionsProvider::contains(std::string name) {
        return this->actions.find(name) != this->actions.end();
    }

    bool ActionsProvider::insert(std::string name, Callback action) {
        if (this->containsAction(name)) {
            return false;
        }

        this->actions.insert(name, action);

        return true;
    }

    std::optional<Callback> ActionsProvider::lookup(std::string name) {
        if (!this->contains(name)) {
            return std::nullopt;
        }

        return this->actions[name];
    }

    void ActionsProvider::registerCommon() {
        this->registerAction("quit", []() {

        });
    }
}
