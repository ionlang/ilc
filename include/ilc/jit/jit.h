#pragma once

#include <string>
#include <map>
#include <optional>
#include <ilc/misc/helpers.h>

namespace ilc::jit {
    static ionshared::Map<std::string, Callback> actions =
        ionshared::Map<std::string, Callback>();

    void registerCommonActions();
}
